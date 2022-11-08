#include <common.h>
#include <utils.h>
#include <device/alarm.h>
#include <SDL2/SDL.h>

void init_serial();
void init_uartlite();
void init_timer();
void init_alarm();

static int device_update_flag = false;

static void set_device_update_flag() {
  device_update_flag = true;
}

void device_update() {
  if (!device_update_flag) {
    return;
  }
  device_update_flag = false;

}

void sdl_clear_event_queue() {


}

void init_device() {
  IFDEF(CONFIG_HAS_SERIAL, init_serial());
  IFDEF(CONFIG_HAS_UARTLITE, init_uartlite());
  IFDEF(CONFIG_HAS_TIMER, init_timer());

  add_alarm_handle(set_device_update_flag);
  init_alarm();
}
