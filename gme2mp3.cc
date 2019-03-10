#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <gme/gme.h>
#include <lame/lame.h>

using namespace std;
namespace fs = std::filesystem;

Music_Emu *gme = nullptr;
lame_global_flags *lame = nullptr;

const int sample_rate = 44100;
const int raw_samples_per_iter = sample_rate;
const int BUFFER_SIZE = 7200 + 2 * raw_samples_per_iter;
short rawbuf[BUFFER_SIZE] = {};
char encbuf[BUFFER_SIZE] = {};

void check_err( const char *err ) {
  if ( err != nullptr ) {
    throw runtime_error( err ); } }

void scan_file ( fs::path src ) {
  cout << src << endl;
  check_err( gme_open_file( src.c_str(), &gme, sample_rate ) );
  gme_enable_accuracy( gme, 1 );
  int tracks = gme_track_count( gme );

  for ( int i = 0; i < tracks; i++ ) {
    auto track_str = tracks == 1 ? "" : to_string(i);    
    auto dst = fs::path(src).replace_extension(track_str + ".mp3");
    if ( fs::exists( dst ) ) {
      continue; }
    cout << "  " << dst;

    gme_info_t *info = nullptr;
    check_err( gme_track_info( gme, &info, i ) );
    auto len = info->play_length;
    // XXX Record info->game and info->song?
    gme_free_info( info );
    if ( len < 0 ) {
      throw runtime_error("Unknown length for track!"); }

    cout << " " << len; 
    check_err( gme_start_track( gme, i ) );

    lame_init_bitstream( lame );    
    
    ofstream fout;
    fout.open( dst, ios::out );
    
    float total_samples = ((float)sample_rate * (float)len) / 1000.0;
    for ( int j = 0; j < total_samples; j += raw_samples_per_iter ) {
      check_err( gme_play( gme, raw_samples_per_iter * 2, rawbuf ) );
      int encwrite = lame_encode_buffer_interleaved( lame, rawbuf, raw_samples_per_iter, (unsigned char *)encbuf, BUFFER_SIZE );
      fout.write( encbuf, encwrite ); }

    int encwrite = lame_encode_flush_nogap( lame, (unsigned char *)encbuf, BUFFER_SIZE );
    fout.write( encbuf, encwrite );
    fout.close();

    cout << endl; }

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
  lame = lame_init();
  lame_set_num_channels(lame, 2);
  lame_set_in_samplerate(lame, sample_rate);
  lame_set_mode(lame, STEREO);
  lame_set_VBR(lame, vbr_default);
  lame_set_VBR_q(lame, 9);
  auto ret_code = lame_init_params(lame);
  if ( ret_code < 0 ) {
    throw runtime_error( "lame_init_params" ); }

  scan_dir( fs::current_path() );

  lame_close( lame );
  
  return 0; }
