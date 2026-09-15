#include <hal/nrf_power.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static void enter_bootloader(void) {
    printk("Entering bootloader\n");

    k_msleep(500);

    nrf_power_gpregret_set(NRF_POWER, 0, 0x57);
    NVIC_SystemReset();
}

static struct gpio_callback button_cb;

static void button_pressed(const struct device *port, struct gpio_callback *cb,
                           gpio_port_pins_t pins) {
    printk("button interrupt\n");
}

int main(void) {
    static const struct gpio_dt_spec led =
        GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

    static const struct gpio_dt_spec button =
        GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button), gpios);

    static const struct device *console =
        DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button) ||
        !device_is_ready(console)) {
        return 0;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT);
    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));
    gpio_add_callback_dt(&button, &button_cb);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

    printk("nerve online\n");

    unsigned char c;
    while (1) {
        if (uart_poll_in(console, &c) == 0) {
            if (c == 'b') {
                enter_bootloader();
            }
        }

        k_msleep(10);
    }
}
