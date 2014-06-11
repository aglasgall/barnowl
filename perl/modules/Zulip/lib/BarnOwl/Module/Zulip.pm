use strict;
use warnings;

package BarnOwl::Module::Zulip;

our $VERSION=0.1;
our $queue_id;
our $last_event_id;
our %cfg;
our $max_retries = 1000;
our $retry_timer;

use AnyEvent;
use AnyEvent::HTTP;
use JSON;
use MIME::Base64;
use URI;
use URI::Encode;
use BarnOwl::Hooks;
use BarnOwl::Message::Zulip;
use HTTP::Request::Common;
use Getopt::Long qw(GetOptionsFromArray);

sub fail {
    my $msg = shift;
    undef %cfg;
    undef $queue_id;
    undef $last_event_id;

    BarnOwl::admin_message('Zulip Error', $msg);
    die("Zulip Error: $msg\n");
}


our $conffile = BarnOwl::get_config_dir() . "/zulip";

if (open(my $fh, "<", "$conffile")) {
    read_config($fh);
    close($fh);
}

sub authorization {
    return "Basic " . encode_base64($cfg{'user'} . ':' . $cfg{'apikey'}, "")
}

sub read_config {
    my $fh = shift;

    my $raw_cfg = do {local $/; <$fh>};
    close($fh);
    eval {
        $raw_cfg = from_json($raw_cfg);
    };
    if($@) {
        fail("Unable to parse $conffile: $@");
    }

    if(! exists $raw_cfg->{user}) {
	fail("Account has no username set.");
    }
    my $user = $raw_cfg->{user};
    if(! exists $raw_cfg->{apikey}) {
	fail("Account has no api key set.");
    }
    my $apikey = $raw_cfg->{apikey};
    if(! exists $raw_cfg->{api_url}) {
	fail("Account has no API url set.");
    }
    my $api_url = $raw_cfg->{api_url};

    if(! exists $raw_cfg->{default_realm}) {
	fail("Account has no default realm set.");
    }
    my $default_realm = $raw_cfg->{default_realm};

    $cfg{'user'} = $user;
    $cfg{'apikey'} = $apikey;
    $cfg{'api_url'} = $api_url;
    $cfg{'realm'} = $default_realm;
}

sub register {
    my $retry_count = 0;
    BarnOwl::debug("In register");
    my $callback;
    $callback = sub {
	BarnOwl::debug("In register callback");
	my ($body, $headers) = @_;
	if($headers->{Status} > 399) {
	    warn "Failed to make event queue registration request ($headers->{Status})";
	    if($retry_count >= $max_retries) {
		fail("Giving up");
	    } else {
		$retry_count++;			      
		http_post($cfg{'api_url'} . "/register", "",
				    headers => { "Authorization" => authorization }, 
				    $callback);
		return;
	    }
	} else {
	    my $response = decode_json($body);
	    if($response->{result} ne "success") {
		fail("Failed to register event queue; error was $response->{msg}");
	    } else {
		$last_event_id = $response->{last_event_id};
		$queue_id = $response->{queue_id};
		do_poll();
		return;
	    }
	}
    };
    
    http_post($cfg{'api_url'} . "/register", "",
	      headers => { "Authorization" => authorization }, $callback);
    return;
}

sub parse_response {
    my ($body, $headers) = @_;
    if($headers->{Status} > 399) {
	return 0;
    }
    my $response = decode_json($body);
    if($response->{result} ne "success") {
	return 0;
    }
    return $response;
}
sub do_poll {
    my $uri = URI->new($cfg{'api_url'} . "/events");
    $uri->query_form("queue_id" => $queue_id, 
		     "last_event_id" => $last_event_id);
    my $retry_count = 0;
    my $callback;
    $callback = sub {
	my ($body, $headers) = @_;
	my $response = parse_response($body, $headers);
	if(!$response) {
	    warn "Failed to poll for events in do_poll: $headers->{Reason}";
	    if($retry_count >= $max_retries) {
		warn "Retry count exceeded in do_poll, giving up";
		fail("do_poll: Giving up");
	    } else {
		warn "Retrying";
		$retry_count++;	      
		$retry_timer = AnyEvent->timer(after => 10, cb => sub { warn "retry number $retry_count"; 
							 http_get($uri->as_string, 
								  "headers" => { "Authorization" => authorization }, 
								  $callback);
							 return;
				});
		return;
	    }
	} else {
	    event_cb($response);
	}
    };
    http_get($uri->as_string, "headers" => { "Authorization" => authorization }, $callback);
    return;
}

sub event_cb {
    my $response = $_[0];
    if($response->{result} ne "success") {
	fail("event_cb: Failed to poll for events; error was $response->{msg}");
    } else {
	for my $event (@{$response->{events}}) {
	    if($event->{type} eq "message") {
		my $msg = $event->{message};
		my %msghash = (
		    type => 'Zulip',
		    sender => $msg->{sender_email},
		    recipient => $msg->{recipient_id},
		    direction => 'in',
		    class => $msg->{display_recipient},
		    instance => $msg->{subject},
		    unix_time => $msg->{timestamp},
		    source => "zulip",
		    location => "zulip",
		    body => $msg->{content},
		    zid => $msg->{id},
		    sender_full_name => $msg->{sender_full_name});
		$msghash{'body'} =~ s/\r//gm;
		if($msg->{type} eq "private") {
		    $msghash{private} = 1;
		    my @raw_recipients = @{$msg->{display_recipient}};
		    my @display_recipients;
		    if (scalar(@raw_recipients) > 1) {
			my $recip;
			for $recip (@raw_recipients) {
			    unless ($recip->{email} eq $cfg{user}) {
				push @display_recipients, $recip->{email};
			    }
			}
			$msghash{recipient} = join " ", @display_recipients;
		    } else {
			$msghash{recipient} = $msg->{display_recipient}->[0]->{email};
		    }
		    $msghash{class} = "message";
		    if($msg->{sender_email} eq $cfg{user}) {
			$msghash{direction} = 'out';
		    }
		}
		my $bomsg = BarnOwl::Message->new(%msghash);
		BarnOwl::queue_message($bomsg);
	    }
	    $last_event_id = $event->{id};
	    do_poll();
	    return;
	}
    }
    
}

sub zulip {
    my ($type, $recipient, $subject, $msg) = @_;
    # only care about it for its url encoding
    my $builder = URI->new("http://www.example.com");
    my %params = ("type" => $type, "to" => $recipient,  "content" => $msg);
    if ($subject ne "") {
    	$params{"subject"} = $subject;
    }
    my $url = $cfg{'api_url'} . "/messages";
    my $req = POST($url, \%params); 
    http_post($url, $req->content, "headers" => {"Authorization" => authorization, "Content-Type" => "application/x-www-form-urlencoded"}, sub { 
	my ($body, $headers) = @_;
	if($headers->{Status} < 400) {
	    BarnOwl::message("Zulipgram sent");
	} else {
	    BarnOwl::message("Error sending zulipgram: $headers->{Reason}!");
	    BarnOwl::debug($body);
	}});
    return;
}

sub cmd_zulip_login {
    register();
}

sub cmd_zulip_write {
    my $cmdline = join " ", @_;
    my $cmd = shift;
    my $stream;
    my $subject;
    my $type;
    my $to;
    my $ret = GetOptionsFromArray(\@_,
			       "c:s" => \$stream,
			       "i:s" => \$subject);
    unless($ret) {
	die("Usage: zulip:write [-c stream] [-i subject] [recipient] ...");
    }
    # anything left is a recipient
    if (scalar(@_) > 0) {
	my @addresses = map {
	    if(/@/) {
		$_;
	    } else {
		$_ . "\@$cfg{'realm'}";
	    }} @_;
	$to = encode_json(\@addresses);
	$type = "private";
	    
    } else {
	$type = "stream";
	$to = $stream
    }
    BarnOwl::start_edit(prompt => $cmdline, type => 'edit_win', 
			callback => sub {
			    my ($text, $should_send) = @_;
			    unless ($should_send) {
				BarnOwl::message("zulip:write cancelled");
				return;
			    }
			    zulip($type, $to, $subject, $text);
			});
    
}

BarnOwl::new_command('zulip:login' => sub { cmd_zulip_login(@_); },
		     {
			 summary => "Log in to Zulip",
			 usage => "zulip:login",
			 description => "Start receiving Zulip messages"
		     });

BarnOwl::new_command('zulip:write' => sub { cmd_zulip_write(@_); },
		     {
			 summary => "Send a zulipgram",
			 usage => "zulip:login [-c stream] [-i subject] [recipient(s)]",
			 description => "Send a zulipgram to a stream, person, or set of people"
		     });

sub default_realm {
  return $cfg{'realm'};
}

1;
