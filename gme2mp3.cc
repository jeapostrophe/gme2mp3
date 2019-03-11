#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <gme/gme.h>
#include <lame/lame.h>

using namespace std;
namespace fs = std::filesystem;

Music_Emu *gme = nullptr;

const int sample_rate = 44100;
const int raw_samples_per_iter = sample_rate;
const int BUFFER_SIZE = 7200 + 2 * raw_samples_per_iter;
short rawbuf[BUFFER_SIZE] = {};
char encbuf[BUFFER_SIZE] = {};

void check_err( const char *err ) {
  if ( err != nullptr ) {
    throw runtime_error( string("gme: ") + err ); } }

void scan_file ( fs::path src ) {
  cout << src << endl;
  check_err( gme_open_file( src.c_str(), &gme, sample_rate ) );
  gme_enable_accuracy( gme, 1 );
  int tracks = gme_track_count( gme );

  bool snes_p = src.extension() == ".spc";

  for ( int i = 0; i < tracks; i++ ) {
    auto lame = lame_init();
    lame_set_num_channels(lame, 2);
    lame_set_in_samplerate(lame, sample_rate);
    lame_set_mode(lame, STEREO);
    lame_set_VBR(lame, vbr_default);
    lame_set_VBR_q(lame, 9);

    id3tag_init( lame );
    id3tag_add_v2( lame );
    id3tag_v2_only( lame );

    gme_info_t *info = nullptr;
    check_err( gme_track_info( gme, &info, i ) );
    auto len = info->play_length;
    if ( len < 0 ) {
      throw runtime_error("Unknown length for track!"); }

    auto fns = string(fs::path(src).filename().replace_extension(""));
    auto fdot = fns.find_first_of(".");
    string song_name = snes_p ? fns.substr(fdot+2) : info->song;
    string game_name = snes_p ? src.parent_path().filename() : info->game;
    auto track_str = (snes_p ? fns.substr(0, fdot) : to_string(i + 1));
    id3tag_set_title( lame, song_name.c_str() );
    id3tag_set_album( lame, game_name.c_str() );
    id3tag_set_artist( lame, snes_p ? "Nintendo SNES" : info->system );
    id3tag_set_track( lame, track_str.c_str() );
    id3tag_set_genre( lame, "Video Game Soundtrack" );

    auto song_name_for_file = string(song_name);
    replace( song_name_for_file.begin(), song_name_for_file.end(), '/', '_' );
    auto new_file =
      (snes_p ? "" : (game_name + "/")) +
      track_str + ". " + song_name_for_file + ".mp3";

    auto dst = fs::path(src).replace_filename(new_file);
    fs::create_directories( dst.parent_path() );
    if ( fs::exists( dst ) ) {
      continue; }
    cout << "  " << game_name << " - " << song_name << endl;

    check_err( gme_start_track( gme, i ) );

    if ( lame_init_params(lame) < 0 ) {
      throw runtime_error( "lame_init_params" ); }

    ofstream fout;
    fout.open( dst, ios::out );

    int total_samples = ceil( ((float)sample_rate * (float)len) / 1000.0 );
    for ( int j = 0; j < total_samples; j += raw_samples_per_iter ) {
      int samples_to_play = min(total_samples - j, raw_samples_per_iter);
      check_err( gme_play( gme, samples_to_play * 2, rawbuf ) );
      int encwrite =
        lame_encode_buffer_interleaved( lame, rawbuf, samples_to_play,
                                        (unsigned char *)encbuf, BUFFER_SIZE );
      fout.write( encbuf, encwrite ); }

    int encwrite =
      lame_encode_flush( lame, (unsigned char *)encbuf, BUFFER_SIZE );
    fout.write( encbuf, encwrite );

    gme_free_info( info );
    fout.close();

    lame_close( lame ); }

  gme_delete( gme ); }

void scan_dir ( fs::path d ) {
  for(auto& p: fs::directory_iterator( d  )) {
    if ( fs::is_directory( p ) ) {
      scan_dir( p ); }
    else {
      auto e = p.path().extension();
      if ( e == ".nsfe" || e == ".nsf" || e == ".spc" ) {
        scan_file( p ); } } } }

int main ( int argc, char **argv ) {
  scan_dir( fs::current_path() );
  return 0; }
