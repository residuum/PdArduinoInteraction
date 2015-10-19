/*
   Parses bytes from Arduino in special format:
   Digital message (1 Byte):
   1 0 0 p p p p v
   Analog message (2 Byte):
   1 1 p p p v v v    0 v v v v v v v

   p = Bits for port
   v = Bits for value

Output: list (a|d) (port as integer) (value as integer)

Copyright (c) 2013-2015 Thomas Mayer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "m_pd.h"
#include <stdio.h>

static t_class *arduino_message_parser_class;

typedef struct _arduino_message_parser {
	t_object x_obj;
	int first_byte;
} t_arduino_message_parser;

void new_byte(t_arduino_message_parser *x, t_floatarg new_value) {
	int prev_byte = x->first_byte;
	int port = 0;
	char port_type = 0;
	int value = 0;
	int new_value_int = new_value;

	if (prev_byte > 0 && (new_value_int & 0x80) == 0) { /* analog, 2nd byte */
		port_type = 'a';
		port = (prev_byte >> 3)  & 7;
		value = ((prev_byte & 7) << 7) + (new_value_int & 0x7f);
		x->first_byte = 0;
	} else if ((new_value_int & 0xc0) == 0x80) { /* digital */
		port_type = 'd';
		port = (new_value_int & 0x7f) >> 1;
		value = new_value_int & 1;
	} else if ((new_value_int & 0xc0) == 0xc0) { /*analog, 1st byte */
		x->first_byte = new_value_int;
	}
	if (port_type == 0) {
		return;
	}
	t_atom out[3];
	char out_data[2];
	out_data[2] = 0x00;
	sprintf(out_data, "%c", port_type);
	SETSYMBOL(&out[0], gensym(out_data));
	SETFLOAT(&out[1], port);
	SETFLOAT(&out[2], value);
	outlet_list(x->x_obj.ob_outlet, &s_list, 3, &out[0]);
}

void *arduino_message_parser_new(void) {
	t_arduino_message_parser *x = (t_arduino_message_parser *)pd_new(arduino_message_parser_class);
	outlet_new(&x->x_obj, &s_list);
	x->first_byte = 0;
	return (void *)x;
}

void arduino_message_parser_setup(void) {
	arduino_message_parser_class = class_new(gensym("arduino_message_parser"),
			(t_newmethod)arduino_message_parser_new,
			0, sizeof(t_arduino_message_parser), 0, 0);
	class_addfloat(arduino_message_parser_class, new_byte);
}

