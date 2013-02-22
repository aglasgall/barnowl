#!/usr/bin/env perl
use strict;
use warnings;

use Test::More qw(no_plan);

use BarnOwl;

# Test the basic variable types.

BarnOwl::new_variable_int("intvar", { default => 7 });
is(BarnOwl::getvar("intvar"), "7", "intvar default");
BarnOwl::set("-q", "intvar", "24");
is(BarnOwl::getvar("intvar"), "24", "intvar set");
BarnOwl::set("-q", "intvar", "string");
is(BarnOwl::getvar("intvar"), "24", "intvar set bogus");
BarnOwl::unset("-q", "intvar");
is(BarnOwl::getvar("intvar"), "24", "intvar unset bogus");

BarnOwl::new_variable_bool("boolvar", { default => 1 });
is(BarnOwl::getvar("boolvar"), "on", "boolvar default");
BarnOwl::set("-q", "boolvar", "off");
is(BarnOwl::getvar("boolvar"), "off", "boolvar set");
BarnOwl::set("-q", "boolvar", "bogus");
is(BarnOwl::getvar("boolvar"), "off", "boolvar set bogus");
BarnOwl::set("-q", "boolvar");
is(BarnOwl::getvar("boolvar"), "on", "boolvar set");
BarnOwl::unset("-q", "boolvar");
is(BarnOwl::getvar("boolvar"), "off", "boolvar unset");

BarnOwl::new_variable_string("strvar", { default => "monkey" });
is(BarnOwl::getvar("strvar"), "monkey", "strvar default");
BarnOwl::set("-q", "strvar", "cuttlefish");
is(BarnOwl::getvar("strvar"), "cuttlefish", "strvar set");
BarnOwl::set("-q", "strvar");
is(BarnOwl::getvar("strvar"), "cuttlefish", "strvar set bogus");
BarnOwl::unset("-q", "strvar");
is(BarnOwl::getvar("strvar"), "cuttlefish", "strvar unset bogus");

BarnOwl::new_variable_enum("enumvar", { validsettings => [qw/foo bar baz/], default => "bar" });
is(BarnOwl::getvar("enumvar"), "bar", "enumvar default");
BarnOwl::set("-q", "enumvar", "baz");
is(BarnOwl::getvar("enumvar"), "baz", "enumvar set");
BarnOwl::set("-q", "enumvar", "bogus");
is(BarnOwl::getvar("enumvar"), "baz", "boolvar set bogus");
BarnOwl::unset("-q", "enumvar");
is(BarnOwl::getvar("enumvar"), "baz", "enumvar unset bogus");

BarnOwl::new_variable_int("intvar2");
is(BarnOwl::getvar("intvar2"), "0", "intvar default default");
BarnOwl::new_variable_bool("boolvar2");
is(BarnOwl::getvar("boolvar2"), "off", "boolvar default default");
BarnOwl::new_variable_string("strvar2");
is(BarnOwl::getvar("strvar2"), "", "strvar default default");
BarnOwl::new_variable_enum("enumvar2", { validsettings => [qw/foo bar baz/] });
is(BarnOwl::getvar("enumvar2"), "foo", "enumvar default default");

# And test new_variable_full
my $value = "foo";
BarnOwl::new_variable_full("fullvar", {
    validsettings => '<short-words>',
    get_tostring => sub { "value is " . $value },
    set_fromstring => sub {
	return -1 unless $_[0] =~ /^...?$/;
	$value = $_[0];
	return 0;
    },
    takes_on_off => 1
});
is(BarnOwl::getvar("fullvar"), "value is foo", "fullvar get");
BarnOwl::set("-q", "fullvar", "bar");
is(BarnOwl::getvar("fullvar"), "value is bar", "fullvar set");
BarnOwl::set("-q", "fullvar");
is(BarnOwl::getvar("fullvar"), "value is on", "fullvar set2");
BarnOwl::unset("-q", "fullvar");
is(BarnOwl::getvar("fullvar"), "value is off", "fullvar unset");
BarnOwl::set("-q", "fullvar", "bogus");
is(BarnOwl::getvar("fullvar"), "value is off", "fullvar set bogus");
$value = "something really long";
is(BarnOwl::getvar("fullvar"), "value is something really long", "fullvar set out-of-band");

1;

