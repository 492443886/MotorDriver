#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include <string.h>
#define LEFT_FORWARD_PIN 2
#define LEFT_BACKWORD_PIN 4


#define RIGHT_FORWARD_PIN 5
#define RIGHT_BACKWORD_PIN 18

void init_pin(void)
{
  // not needed for version 5
  // gpio_pad_select_gpio(LED);
  gpio_set_direction(LEFT_FORWARD_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(LEFT_BACKWORD_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(RIGHT_FORWARD_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(RIGHT_BACKWORD_PIN, GPIO_MODE_OUTPUT);
}

void toggle_pin(char* lr, int speed)
{



    if(strcmp(lr, "left") == 0){


        printf("speedl: %d\n", speed);
        if(speed == 0){
            gpio_set_level(LEFT_FORWARD_PIN, 0);
            gpio_set_level(LEFT_BACKWORD_PIN, 0);
        }else if(speed > 0){
            gpio_set_level(LEFT_FORWARD_PIN, 1);
            gpio_set_level(LEFT_BACKWORD_PIN, 0);
        }else if(speed < 0){
            gpio_set_level(LEFT_FORWARD_PIN, 0);
            gpio_set_level(LEFT_BACKWORD_PIN, 1);
        }

    }else if (strcmp(lr, "right") == 0)
    {

        printf("speedr: %d\n", speed);
        if(speed == 0){
            gpio_set_level(RIGHT_FORWARD_PIN, 0);
            gpio_set_level(RIGHT_BACKWORD_PIN, 0);
        }else if(speed > 0){
            gpio_set_level(RIGHT_FORWARD_PIN, 1);
            gpio_set_level(RIGHT_BACKWORD_PIN, 0);
        }else if(speed < 0){
            gpio_set_level(RIGHT_FORWARD_PIN, 0);
            gpio_set_level(RIGHT_BACKWORD_PIN, 1);
        }
    }
}

