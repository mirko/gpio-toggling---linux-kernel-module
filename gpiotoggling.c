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

static int start = 0;
static int stop = 0;
static unsigned int count = 1; // DANGEROUS - 0 will end up in an infinite loop and stall your system
static unsigned int interval = 0; // long
static char loop = 0; // DANGEROUS - >=0 will end up in an infinite loop and stall your system
static unsigned int verbosity = 0; // u8 // TODO: implement event logging handling in some way

static char lock = 0; // u8

int gpiotoggling(void) {
	int i;
	for (i=start; i<=stop; i++)
	{
		int valid = gpio_is_valid(i);
		if (valid) {
			printk(KERN_ALERT "GPIO <%i> is meant to be valid - configuring as output pin\n", i);
			gpio_direction_output(i, 0);
			int ii;
			for (ii=0; (ii<count || count==0); ii++) {
				ndelay(interval);
				//printk(KERN_ALERT "  set to high");
				gpio_set_value(i, 1);
				ndelay(interval);
				//printk(KERN_ALERT "  set to low");
				gpio_set_value(i, 0);
			}
		}
	}
	return 0;
}


struct device *tmp_dev;

static ssize_t start_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: start\n");
	return sprintf(buf, "%i\n", start);
}
static ssize_t stop_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: stop\n");
	return sprintf(buf, "%i\n", stop);
}
static ssize_t count_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: count\n");
	return sprintf(buf, "%u\n", count);
}
static ssize_t interval_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: interval\n");
	return sprintf(buf, "%u\n", interval);
}
static ssize_t loop_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: loop\n");
	printk("FOOOOOOOO\n");
	return sprintf(buf, "%i\n", loop);
}
static ssize_t verbosity_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	printk("show: verbosity\n");
	return sprintf(buf, "%u\n", verbosity);
}
//static ssize_t fire_show(struct kobject *kobj, struct attribute *attr, char *buf)
//{
//	printk("show: fire\n");
//	return -EINVAL;
//}


//TODO: check length and return values
static ssize_t start_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: start\n");
	strict_strtol(buf, 10, &start);
	return len;
}
static ssize_t stop_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: stop\n");
	strict_strtol(buf, 10, &stop);
	return len;
}
static ssize_t count_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: count\n");
	strict_strtoul(buf, 10, &count);
	return len;
}
static ssize_t interval_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: interval\n");
	strict_strtoul(buf, 10, &interval);
	return len;
}
static ssize_t loop_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: loop\n");
	strict_strtol(buf, 10, &loop);
	return len;
}
static ssize_t verbosity_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	printk("store: verbosity\n");
	strict_strtol(buf, 10, &verbosity);
	return len;
}
static ssize_t fire_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	lock = 0;
	printk("store: fire\n");
	static u8 fire;
	strict_strtoul(buf, 10, &fire);
	if ((fire != 1) || (lock))
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

static DEVICE_ATTR(start, 0644, start_show, start_store);
static DEVICE_ATTR(stop, 0644, stop_show, stop_store);
static DEVICE_ATTR(count, 0644, count_show, count_store);
static DEVICE_ATTR(interval, 0644, interval_show, interval_store);
static DEVICE_ATTR(loop, 0644, loop_show, loop_store);
static DEVICE_ATTR(verbosity, 0644, verbosity_show, verbosity_store);
static DEVICE_ATTR(fire, 0644, NULL, fire_store);



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
