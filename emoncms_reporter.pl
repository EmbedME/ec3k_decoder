#!/usr/bin/perl -w

# Report decoding result to emoncms.org

use LWP::UserAgent;

my $apikey = "INSERT_YOUR_EMONCMS_APIKEY_HERE";

open(DECODER, "rtl_fm -f 868402000 -s 200000 -A fast - | ./ec3k_decoder |");

while (my $line = <DECODER>) {

  @values = split(',', $line);

  my $id = $values[1];
  my $id_power = $id . "_power";
  my $id_kwh = $id . "_kwh";

    my $power = $values[2] / 10;
    my $kwh = $values[3] / 60 / 60;

    my $emon_ua = LWP::UserAgent->new;
    my $emon_url = "http://emoncms.org/input/post.json?json={$id_power:$power,$id_kwh:$kwh}&apikey=$apikey";
    print "$emon_url ...";

    my $emon_response = $emon_ua->get($emon_url);

    if ($emon_response->is_error) {
      print " Error\n";
    } else {
      print " OK\n";
    }
}
