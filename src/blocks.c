#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xDE, 0xBC, 0x27, 0x8B, 0xC2, 0x01, 0x40, 0x75, 0x9F, 0xC2, 0x79, 0x25, 0xAC, 0xBC, 0xC9, 0x39 }
PBL_APP_INFO(MY_UUID,
             "Blocks", "Charles Randall",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer layer;

#define BACKCOLOR GColorBlack
#define FORECOLOR GColorWhite

#define HOURSTARTX 1
#define HOURSTARTY 20

#define HOURGAP 2
#define HOURSIZE 22

#define TENSSTARTX 12
#define TENSSTARTY 70

#define TENSGAP 2
#define TENSSIZE 22

long seed = 0;
// modified from http://forums.getpebble.com/discussion/4456/random-number-generation
int random(int mod)
{
  seed = (((seed * 214013L + 2531011L) >> 16) & 32767);

  return ((seed % mod));
}

void fill_random_bit( unsigned short *pbits, char* pbitsset, char max, char cur )
{
  if( cur < *pbitsset ) // we've looped, clear it out
  {
    *pbits = 0;
    *pbitsset = 0;
  }
  while( *pbitsset < cur )
  {
    short r = 1 << random(max);

    if( *pbits & r ) //bit is set
    {

      if( random(2) == 1 )
      {
TESTDOWN:
        while( r > 1 )
        {
          r >>= 1;

          if( (*pbits & r) == 0 )
          {
            goto DONE;
          }
        }
        goto TESTUP;
      }
      else
      {
TESTUP:
        while( r < 1 << (max-1))
        {
          r <<= 1;

          if( (*pbits & r) == 0 )
          {
            goto DONE;
          }
        }
        goto TESTDOWN;
      }
    }
DONE:
    *pbits |= r;
    (*pbitsset)++;
  }
}

void draw_blocks(GContext *ctx, int x, int y, unsigned short bits, char max, int gap, int size)
{
  for (int i = 0; i <= max; ++i)
  {

    if( bits & (1<<i) )
    {
      graphics_fill_rect(ctx, GRect( x, y, size, size), 0, GCornerTopLeft);
    }

    x += gap + size;
  }

}

void layer_update_callback(Layer *me, GContext* ctx) 
{
  (void)me;


  graphics_context_set_stroke_color(ctx, BACKCOLOR);
  graphics_context_set_fill_color(ctx, BACKCOLOR);

  graphics_fill_rect(ctx, GRect( 0,0, 144,168), 0, GCornerTopLeft);

  static unsigned short hourbits = 0;
  static char hourbitsset = 0;

  static unsigned short tensbits = 0;
  static char tensbitsset = 0;

  static unsigned short minbits = 0;
  static char minbitsset = 0;

  PblTm t;

  get_time(&t);

  unsigned short hour = t.tm_hour % 12;
  hour = hour ? hour : 12;
  unsigned short tens = t.tm_min / 10;
  unsigned short mins = t.tm_min % 10;

  fill_random_bit(&hourbits, &hourbitsset, 12, hour);
  fill_random_bit(&tensbits, &tensbitsset, 5, tens);
  fill_random_bit(&minbits, &minbitsset, 9, mins);

  graphics_context_set_stroke_color(ctx, FORECOLOR);
  graphics_context_set_fill_color(ctx, FORECOLOR);

  draw_blocks(ctx, HOURSTARTX, HOURSTARTY, hourbits & (0x7F), 6, HOURGAP, HOURSIZE);
  draw_blocks(ctx, HOURSTARTX, HOURSTARTY + HOURGAP + HOURSIZE, hourbits >> 6, 6, HOURGAP, HOURSIZE);

  draw_blocks(ctx, TENSSTARTX, 90, tensbits, 5, TENSGAP, TENSSIZE);

  draw_blocks(ctx, 1, 132, minbits, 9, TENSGAP, 14);

}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)ctx;

  layer_mark_dirty(&layer);
}

 
void handle_init(AppContextRef ctx) 
{
  (void)ctx;

  window_init(&window, "Filler");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, BACKCOLOR);

  layer_init(&layer, window.layer.frame );
  layer_add_child(&window.layer, &layer);
  layer.update_proc = layer_update_callback;


  PblTm t;

  get_time(&t);

  seed = t.tm_sec | t.tm_min << 5 | t.tm_hour << 8;

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
