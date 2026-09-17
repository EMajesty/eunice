#include <hal/nrf_power.h>
#include <inttypes.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct nerve_button {
    struct gpio_dt_spec gpio;
    struct gpio_callback callback;
    size_t index;
    volatile uint32_t count;
};

static void enter_bootloader(void) {
    printk("Entering bootloader\n");

    k_msleep(500);

    nrf_power_gpregret_set(NRF_POWER, 0, 0x57);
    NVIC_SystemReset();
}

static void button_pressed(const struct device *port, struct gpio_callback *cb,
                           gpio_port_pins_t pins) {
    struct nerve_button *button =
        CONTAINER_OF(cb, struct nerve_button, callback);
    button->count++;
}

int main(void) {
    static const struct gpio_dt_spec led =
        GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

    static struct nerve_button buttons[] = {
        {
            .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button1), gpios),
            .index = 1,
        },
        {
            .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button2), gpios),
            .index = 2,
        },
        {
            .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button3), gpios),
            .index = 3,
        },
        {
            .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button4), gpios),
            .index = 4,
        },
        {
            .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(nerve_button5), gpios),
            .index = 5,
        },
    };

    // static struct gpio_callback button_cbs[ARRAY_SIZE(buttons)];

    static const struct device *console =
        DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (!gpio_is_ready_dt(&led) || !device_is_ready(console)) {
        return 0;
    }

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
        if (!gpio_is_ready_dt(&buttons[i].gpio)) {
            return 0;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
        gpio_pin_configure_dt(&buttons[i].gpio, GPIO_INPUT);
        gpio_init_callback(&buttons[i].callback, button_pressed,
                           BIT(buttons[i].gpio.pin));
        gpio_add_callback_dt(&buttons[i].gpio, &buttons[i].callback);
        gpio_pin_interrupt_configure_dt(&buttons[i].gpio,
                                        GPIO_INT_EDGE_TO_ACTIVE);
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    printk("nerve online\n");

    unsigned char c;
    int64_t time = k_uptime_get();

    while (1) {
        if (uart_poll_in(console, &c) == 0) {
            if (c == 'b') {
                enter_bootloader();
            }
        }
        int64_t elapsed = k_uptime_get() - time;
        if (elapsed > 5000) {
            for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
                printk("%zu=%" PRIu32 " ", i, buttons[i].count);
            }
            printk("\n");
            time = k_uptime_get();
        }

        k_msleep(10);
    }
}
