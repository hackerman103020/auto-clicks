#include <string.h>
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb_hid.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdio.h>
#include "version.h"
#include "Keycodes.h"



// Uncomment to be able to make a screenshot
//#define USB_HID_AUTOFIRE_SCREENSHOT

typedef enum {
  EventTypeInput,
}
EventType;

typedef struct {
  union {
    InputEvent input;
  };
  EventType type;
}
UsbMouseEvent;

bool btn_left_autofire = false;
bool btn_right_autofire = false;
bool ison = false;
float autofire_delay = 500.0000000;
uint32_t autofire_delays = 1;

static void usb_hid_autofire_render_callback(Canvas * canvas, void * ctx) {
  UNUSED(ctx);
  char autofire_delay_str[12];
  itoa(autofire_delays, autofire_delay_str, 10);

  canvas_clear(canvas);

  canvas_set_font(canvas, FontPrimary);
  canvas_draw_str(canvas, 0, 10, "LFT/RHT AUTOCLICKER"); //16 charecters long
  canvas_draw_str(canvas, 0, 34, ison ? "<active>" : "<inactive>");

  canvas_set_font(canvas, FontSecondary);
  canvas_draw_str(canvas, 55, 34, "up/down = time");
  canvas_draw_str(canvas, 0, 22, "press [ok] for off");
  canvas_draw_str(canvas, 0, 45, "CPS:               ");
  canvas_draw_str(canvas, 40, 46, autofire_delay_str);
  canvas_draw_str(canvas, 0, 63, "Left/Right for click");
}

static void usb_hid_autofire_input_callback(InputEvent * input_event, void * ctx) {
  FuriMessageQueue * event_queue = ctx;

  UsbMouseEvent event;
  event.type = EventTypeInput;
  event.input = * input_event;
  furi_message_queue_put(event_queue, & event, FuriWaitForever);
}

int32_t usb_hid_autofire_app(void * p) {
  UNUSED(p);
  FuriMessageQueue * event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));
  furi_check(event_queue);
  ViewPort * view_port = view_port_alloc();

  FuriHalUsbInterface * usb_mode_prev = furi_hal_usb_get_config();
  #ifndef USB_HID_AUTOFIRE_SCREENSHOT
  furi_hal_usb_unlock();
  furi_check(furi_hal_usb_set_config( & usb_hid, NULL) == true);
  #endif

  view_port_draw_callback_set(view_port, usb_hid_autofire_render_callback, NULL);
  view_port_input_callback_set(view_port, usb_hid_autofire_input_callback, event_queue);

  // Open GUI and register view_port
  Gui * gui = furi_record_open(RECORD_GUI);
  gui_add_view_port(gui, view_port, GuiLayerFullscreen);

  UsbMouseEvent event;
  while (1) {
    FuriStatus event_status = furi_message_queue_get(event_queue, & event, 50);

    if (event_status == FuriStatusOk) {
      if (event.type == EventTypeInput) {
        if (event.input.key == InputKeyBack) {
          break;
        }

        if (event.input.type != InputTypeRelease) {
          continue;
        }

        switch (event.input.key) {
        case InputKeyOk:
        btn_left_autofire = false;
        btn_right_autofire = false;
            ison = false;
            
          break;
        case InputKeyUp:
            
            if (autofire_delays < 50) {
            autofire_delays += 1;
          } 
            
          break;
        case InputKeyDown:
         if (autofire_delays > 0) {
            autofire_delays -= 1;
          }
          break;
        case InputKeyLeft:
 autofire_delay = ((1000/autofire_delays) / 2);
if (btn_left_autofire == false){
    if (btn_right_autofire == true){
        btn_right_autofire = false;
        btn_left_autofire = true;
    }
    else {
         btn_left_autofire = true;
    }
    ison = true;
    
}
            
          break;
        case InputKeyRight:
 autofire_delay = ((1000/autofire_delays) / 2);
if (btn_right_autofire == false){
    if (btn_left_autofire == true){
        btn_left_autofire = false;
        btn_right_autofire = true;
    }
    else {
         btn_right_autofire = true;
    }
}
            ison = true;
            
          break;
        default:
          break;
        }
      }
    }

      if(btn_left_autofire) {
            furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
            // TODO: Don't wait, but use the timer directly to just don't send the release event (see furi_hal_cortex_delay_us)
            furi_delay_us(500000);
            furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
            furi_delay_us(500000);
        }
      if(btn_right_autofire) {
            furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
            // TODO: Don't wait, but use the timer directly to just don't send the release event (see furi_hal_cortex_delay_us)
            furi_delay_us(autofire_delay);
            furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
            furi_delay_us(autofire_delay);
        }


    view_port_update(view_port);
  }

  furi_hal_usb_set_config(usb_mode_prev, NULL);

  // remove & free all stuff created by app
  gui_remove_view_port(gui, view_port);
  view_port_free(view_port);
  furi_message_queue_free(event_queue);

  return 0;
}
