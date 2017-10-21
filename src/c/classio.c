#include <pebble.h>


static Window *window;
static TextLayer *s_time_layer;
static TextLayer *s_sec_layer;


static TextLayer *s_date_layer;
TextLayer *text_temp_layer;
TextLayer *degree_layer;
static GFont s_time_font;
static int s_battery_level;
static Layer *s_battery_laye;


#define NUMBER_OF_IMAGES 11
static GBitmap *image = NULL;
static BitmapLayer *image_layer;

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_CLEAR_DAY,
  RESOURCE_ID_CLEAR_NIGHT,
  RESOURCE_ID_PARTLY_CLOUDY_DAY,
  RESOURCE_ID_PARTLY_CLOUDY_NIGHT,
  RESOURCE_ID_CLOUDY,
  RESOURCE_ID_FOG,
  RESOURCE_ID_RAIN,
  RESOURCE_ID_SLEET,
  RESOURCE_ID_SNOW,
  RESOURCE_ID_WIND,
  RESOURCE_ID_ERROR
};

enum {
  WEATHER_TEMPERATURE_F,
  WEATHER_TEMPERATURE_C,
  WEATHER_ICON,
  WEATHER_ERROR,
  LOCATION
};


void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  Tuple *temperature = dict_find(received, WEATHER_TEMPERATURE_C);
  Tuple *icon = dict_find(received, WEATHER_ICON);

  if (temperature) {
    text_layer_set_text(text_temp_layer, temperature->value->cstring);
    text_layer_set_text(degree_layer, "°");
  }

  if (icon) {
    // figure out which resource to use
    int8_t id = icon->value->int8;
    if (image != NULL) {
      gbitmap_destroy(image);
      layer_remove_from_parent(bitmap_layer_get_layer(image_layer));
      bitmap_layer_destroy(image_layer);
    }

    Layer *window_layer = window_get_root_layer(window);

    image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[id]);
    image_layer = bitmap_layer_create(GRect(10, 84, 60, 60));
    bitmap_layer_set_bitmap(image_layer, image);
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00";
  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);
  
  
  
  static char s_date_text[] = "01.01.";
  strftime(s_date_text, sizeof(s_date_text), "%d.%m.", tick_time);
  
  static char s_sec_text[] = "00";
  strftime(s_sec_text, sizeof(s_sec_text), "%S", tick_time);
  

  
  text_layer_set_text(s_time_layer, s_time_text);
  
  text_layer_set_text(s_sec_layer, s_sec_text);

  text_layer_set_text(s_date_layer, s_date_text);
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  
  // Update meter
  layer_mark_dirty(s_battery_laye);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar (total width = 114px)
  int width = (s_battery_level * 144) / 100;

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

// we need the declaration below to handle calling prv_accel_tap_handler() before it's been defined.
static void prv_accel_tap_handler(AccelAxisType axis, int32_t direction) ;

// we need the declaration below to handle calling prv_accel_tap_handler() before it's been defined.
static void prv_accel_tap_handler(AccelAxisType axis, int32_t direction) ;

static void prv_app_timer_callback(void *data) {
    layer_set_hidden(text_layer_get_layer(s_sec_layer), true);
    accel_tap_service_subscribe(prv_accel_tap_handler);
}

static void prv_accel_tap_handler(AccelAxisType axis, int32_t direction) {
    accel_tap_service_unsubscribe();
    layer_set_hidden(text_layer_get_layer(s_sec_layer), false);
    app_timer_register(5000, prv_app_timer_callback, NULL); // timeout is in milliseconds and we don't have any data to pass to the callback
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_battery_laye = layer_create(GRect(0, 79, 190, 2));
layer_set_update_proc(s_battery_laye, battery_update_proc);

// Add to Window
layer_add_child(window_get_root_layer(window), s_battery_laye);
  

   s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_48));
  
  
  s_time_layer = text_layer_create(GRect(0, 10, bounds.size.w, 50));
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_background_color(s_time_layer, GColorClear);

  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  s_sec_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));
  text_layer_set_text_color(s_sec_layer, GColorBlack);
  text_layer_set_background_color(s_sec_layer, GColorClear);
  layer_set_hidden(text_layer_get_layer(s_sec_layer), true);

   accel_tap_service_subscribe(prv_accel_tap_handler);
  

  text_layer_set_text_alignment(s_sec_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_sec_layer));
  text_layer_set_font(s_sec_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

  

  
  text_temp_layer = text_layer_create(GRect(56, 85, 144-80, 168-108));
  text_layer_set_text_color(text_temp_layer, GColorBlack);
  text_layer_set_background_color(text_temp_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(text_temp_layer));
  text_layer_set_font(text_temp_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(text_temp_layer, GTextAlignmentRight);
  
  degree_layer = text_layer_create(GRect(118, 85, 144-80, 168-108));
  text_layer_set_text_color(degree_layer, GColorBlack);
  text_layer_set_background_color(degree_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(degree_layer));
  text_layer_set_font(degree_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  

  s_date_layer = text_layer_create(GRect(0, 140, bounds.size.w, 50));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  

   


  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  battery_state_service_subscribe(battery_callback);
  

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_sec_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  battery_callback(battery_state_service_peek());

  
}

static void main_window_unload(Window *window) {
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_sec_layer);
  
  layer_destroy(s_battery_laye);
  text_layer_destroy(s_date_layer);

}

static void app_message_init(void) {
  app_message_open(64, 16);
  app_message_register_inbox_received(in_received_handler);
}

static void init() {
  window = window_create();
  app_message_init();
  window_set_background_color(window, GColorLightGray);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(window, true);
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();

  window_stack_remove(window, true);

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
