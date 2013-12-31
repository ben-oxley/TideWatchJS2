#include "pebble.h"

static Window *window;

static TextLayer *temperature_layer;
static TextLayer *city_layer;
static Layer *line_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
#define SYNC_BUFFER_SIZE 120
static uint8_t sync_buffer[SYNC_BUFFER_SIZE];
static uint32_t bufferPos = 0;
const uint32_t FEET_TO_MM = 305;
uint32_t timeArr[4];
uint32_t heightArr[4];


#define MAX_ANGLE   0x10000




int32_t tide_calc(time_t);
static void update_tide_array(uint32_t addHeight);
static void update_time_array(uint32_t addTime);

enum WeatherKey {
  WEATHER_POSN_KEY = 0x0,          // TUPLE_INT32
  WEATHER_TIME_KEY = 0x1,         // TUPLE_INT32
  WEATHER_TIDE_KEY = 0x2,         // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x3         // TUPLE_CSTRING
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  uint32_t tempHeight;
  uint32_t tempTime;
  char* sTempHeight;
  sTempHeight = (char*)malloc(sizeof(char) * 10);
  switch (key) {
    case WEATHER_TIME_KEY:
      tempTime = new_tuple->value->int32;
      APP_LOG(APP_LOG_LEVEL_INFO,"Timestamp: %ld",tempTime);
      update_time_array(tempTime);
      break;

    case WEATHER_TIDE_KEY:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      strcpy(sTempHeight ,new_tuple->value->cstring);
      tempHeight = atoi(sTempHeight);
      APP_LOG(APP_LOG_LEVEL_INFO,"Tide Height in MM: %ld",tempHeight);
      update_tide_array(tempHeight);
      snprintf(sTempHeight,10,"%ldmm",tempHeight);
      text_layer_set_text(temperature_layer, sTempHeight);
      break;

    case WEATHER_CITY_KEY:
      text_layer_set_text(city_layer, new_tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_INFO,"City: %s",new_tuple->value->cstring);
      break;

    case WEATHER_POSN_KEY:
      bufferPos = new_tuple->value->int32;
      APP_LOG(APP_LOG_LEVEL_INFO,"BufferPosition: %ld",bufferPos);
      break;
  }
  
  free(sTempHeight);
}

static void update_tide_array(uint32_t addHeight) {
  heightArr[bufferPos] = addHeight;
      APP_LOG(APP_LOG_LEVEL_INFO,"Logged Height to BufferPosition: %ld",bufferPos);
    //Call update graph;

}

static void update_time_array(uint32_t addTime) {
  timeArr[bufferPos] = addTime;

 APP_LOG(APP_LOG_LEVEL_INFO,"Logged time to BufferPosition: %ld",bufferPos);
}

void line_layer_update_callback(Layer *layer, GContext* ctx) {

  graphics_context_set_fill_color(ctx, GColorWhite);
  int i = 0;
  int magnitude=0;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (i = 0;i<120;i++) {
    magnitude = 10.0*sin_lookup(MAX_ANGLE*i/100)/MAX_ANGLE;
    //magnitude = tide_calc(now + i*360)/20;
    //if (i < 2*second_angle) {
      
    if ( (i%10) > 5) {
      
      graphics_draw_line(ctx, GPoint(i, 50), GPoint(i, 50-2*magnitude));
    } else {
      if ( (i%2) > 0) {
      graphics_draw_line(ctx, GPoint(i, 50), GPoint(i, 50-magnitude));
      }
    }
    
  
      graphics_draw_line(ctx, GPoint(i, 51-2*magnitude), GPoint(i, 50-2*magnitude));
  }

}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  icon_layer = bitmap_layer_create(GRect(32, 10, 80, 80));
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));

  temperature_layer = text_layer_create(GRect(0, 70, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  city_layer = text_layer_create(GRect(0, 100, 144, 80));
  text_layer_set_text_color(city_layer, GColorWhite);
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_font(city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(city_layer));

  line_layer = layer_create(GRect(0, 0, 144, 100));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);

  Tuplet initial_values[] = {
    TupletInteger(WEATHER_POSN_KEY, 0),
    TupletInteger(WEATHER_TIME_KEY, 1),
    TupletCString(WEATHER_TIDE_KEY, "1234 ft"),
    TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }

  text_layer_destroy(city_layer);
  text_layer_destroy(temperature_layer);
  bitmap_layer_destroy(icon_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = SYNC_BUFFER_SIZE;
  const int outbound_size = SYNC_BUFFER_SIZE;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
