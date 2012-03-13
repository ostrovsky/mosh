/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <errno.h>
#include <locale.h>
#include <string.h>
#include <langinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#if HAVE_PTY_H
#include <pty.h>
#elif HAVE_UTIL_H
#include <util.h>
#endif

extern "C" {
#include "selfpipe.h"
}

#include "swrite.h"
#include "completeterminal.h"
#include "user.h"
#include "terminaloverlay.h"

const int ITERATIONS = 100000;

int main( void )
{
  Terminal::Framebuffer local_framebuffer( 80, 24 );
  Overlay::OverlayManager overlays;
  Terminal::Display display( true );
  Terminal::Complete local_terminal( 80, 24 );

  /* Adopt native locale */
  if ( NULL == setlocale( LC_ALL, "" ) ) {
    perror( "setlocale" );
    exit( 1 );
  }

  /* Verify locale calls for UTF-8 */
  if ( strcmp( nl_langinfo( CODESET ), "UTF-8" ) != 0 ) {
    fprintf( stderr, "mosh requires a UTF-8 locale.\n" );
    exit( 1 );
  }

  for ( int i = 0; i < ITERATIONS; i++ ) {
    /* type a character */
    overlays.get_prediction_engine().new_user_byte( 'x', local_framebuffer );

    /* fetch target state */
    Terminal::Framebuffer new_state( local_terminal.get_fb() );

    /* apply local overlays */
    overlays.apply( new_state );

    /* calculate minimal difference from where we are */
    const string diff( display.new_frame( false,
					  local_framebuffer,
					  new_state ) );    

    /* make sure to use diff */
    if ( diff.size() > INT_MAX ) {
      exit( 1 );
    }

    local_framebuffer = new_state;
  }

  return 0;
}
