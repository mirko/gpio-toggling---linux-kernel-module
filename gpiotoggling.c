/*
 * gpiotoggling.c - GPIO toggling in various ways configureable
 *                  via userspace
 *
 * Copyright (C) 2010 Mirko Vogt <dev@nanl.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


//TODO: not all includes are needed
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <asm/uaccess.h>
#include <linux/kobject.h>

static struct kobject *dev;

static long int start = 0;
static long int stop = 0;
static unsigned long int count = 1; // DANGEROUS - 0 will end up in an infinite loop and stall your system
static unsigned long int interval = 1000; // long
static long int loop = 0; // DANGEROUS - >=0 will end up in an infinite loop and stall your system
static unsigned long int verbosity = 0; // u8 // TODO: implement event logging handling in some way

static char lock = 0; // u8

int pin_on(int pin) {
    int valid = gpio_is_valid(pin);
    if (valid) {
        int ii;
        printk(KERN_ALERT "GPIO <%i> is meant to be valid - configuring as output pin\n", pin);
        gpio_direction_output(pin, 0);
        if (verbosity) printk(KERN_ALERT "  set to high\n");
        gpio_set_value(pin, 1);
    }
    return valid;
}

int pin_off(int pin) {
    int valid = gpio_is_valid(pin);
    if (valid) {
        int ii;
        printk(KERN_ALERT "GPIO <%i> is meant to be valid - configuring as output pin\n", pin);
        gpio_direction_output(pin, 0);
        if (verbosity) printk(KERN_ALERT "  set to low\n");
        gpio_set_value(pin, 0);
    }
}

int gpiotoggling(void) {
	int i;
	for (i=start; i<=stop; i++)
	{
        if (pin_on(i)) {
				ndelay(interval);
                pin_off(i);
        }
	}
	return 0;
}


struct device *tmp_dev;

static ssize_t start_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: start\n");
	return sprintf(buf, "%li\n", start);
}
static ssize_t stop_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: stop\n");
	return sprintf(buf, "%li\n", stop);
}
static ssize_t count_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: count\n");
	return sprintf(buf, "%lu\n", count);
}
static ssize_t interval_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: interval\n");
	return sprintf(buf, "%lu\n", interval);
}
static ssize_t loop_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: loop\n");
	printk("FOOOOOOOO\n");
	return sprintf(buf, "%li\n", loop);
}
static ssize_t verbosity_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: verbosity\n");
	return sprintf(buf, "%lu\n", verbosity);
}
//static ssize_t fire_show(struct kobject *kobj, struct attribute *attr, char *buf)
//{
//	printk("show: fire\n");
//	return -EINVAL;
//}


//TODO: check length and return values
static ssize_t start_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: start\n");
	r = strict_strtol(buf, 10, &start);
	return r ? 0 : len;
}
static ssize_t stop_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: stop\n");
	r = strict_strtol(buf, 10, &stop);
	return r ? 0 : len;
}
static ssize_t count_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: count\n");
	r = strict_strtoul(buf, 10, &count);
	return r ? 0 : len;
}
static ssize_t interval_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: interval\n");
	r = strict_strtoul(buf, 10, &interval);
	return r ? 0 : len;
}
static ssize_t loop_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: loop\n");
	r = strict_strtol(buf, 10, &loop);
	return r ? 0 : len;
}
static ssize_t verbosity_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    int r;
	printk("store: verbosity\n");
	r = strict_strtol(buf, 10, &verbosity);
	return r ? 0 : len;
}
static ssize_t fire_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    unsigned long fire;
    int r;
	lock = 0;
	printk("store: fire\n");
	r = strict_strtoul(buf, 10, &fire);
	if (r || (fire != 1) || (lock))
		return -EINVAL;
	else {
		do {
			printk(KERN_ALERT "start toggling\n");
			gpiotoggling();
		} while((int)loop);
	}
	lock = 0;
	return len;
}

static ssize_t pin_on_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    unsigned long pin;
    int r;
	lock = 0;
	printk("store: pin_on\n");
	r = strict_strtoul(buf, 10, &pin);
	if (r || (lock))
		return -EINVAL;
	else {
		pin_on(pin);
	}
	lock = 0;
	return len;
}

static ssize_t pin_off_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    unsigned long pin;
    int r;
	lock = 0;
	printk("store: pin_off\n");
	r = strict_strtoul(buf, 10, &pin);
	if (r || (lock))
		return -EINVAL;
	else {
		pin_off(pin);
	}
	lock = 0;
	return len;
}

static DEVICE_ATTR(start, 0644, start_show, start_store);
static DEVICE_ATTR(stop, 0644, stop_show, stop_store);
static DEVICE_ATTR(count, 0644, count_show, count_store);
static DEVICE_ATTR(interval, 0644, interval_show, interval_store);
static DEVICE_ATTR(loop, 0644, loop_show, loop_store);
static DEVICE_ATTR(verbosity, 0644, verbosity_show, verbosity_store);
static DEVICE_ATTR(fire, 0644, NULL, fire_store);
static DEVICE_ATTR(pin_on, 0644, NULL, pin_on_store);
static DEVICE_ATTR(pin_off, 0644, NULL, pin_off_store);

int init_module(void)
{
    //TODO: check for return values and handle errors
	dev = kobject_create_and_add("gpiotoggling", NULL);

    //TODO: check for return values and handle errors
	sysfs_create_file(dev, &dev_attr_start);
	sysfs_create_file(dev, &dev_attr_stop);
	sysfs_create_file(dev, &dev_attr_count);
	sysfs_create_file(dev, &dev_attr_interval);
	sysfs_create_file(dev, &dev_attr_loop);
	sysfs_create_file(dev, &dev_attr_verbosity);
	sysfs_create_file(dev, &dev_attr_fire);
	sysfs_create_file(dev, &dev_attr_pin_on);
	sysfs_create_file(dev, &dev_attr_pin_off);

	return 0;
}

void cleanup_module(void)
{
    //TODO: check for return values and handle errors
	sysfs_remove_file(dev, &dev_attr_start);
	sysfs_remove_file(dev, &dev_attr_stop);
	sysfs_remove_file(dev, &dev_attr_count);
	sysfs_remove_file(dev, &dev_attr_interval);
	sysfs_remove_file(dev, &dev_attr_loop);
	sysfs_remove_file(dev, &dev_attr_verbosity);
	sysfs_remove_file(dev, &dev_attr_fire);

	kobject_del(dev);
}

MODULE_LICENSE("GPL");
