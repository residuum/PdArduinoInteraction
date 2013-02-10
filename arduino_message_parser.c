/*
Parses bytes from Arduino in special format:
Digital message (1 Byte):
1 0 0 p p p p v
Analog message (2 Byte):
0 0 p p p v v v    1 v v v v v v v

p = Bits for port
v = Bits for value

Output: list (a|d) (port as integer) (value as integer)
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
	char port_type;
	int value = 0;
	int new_value_int = (int)new_value;

	if (prev_byte > 0) {
		port_type = 'a';
		port = prev_byte >> 3;
		value = ((prev_byte & 7) << 7) + (new_value_int & 127);
		x->first_byte = 0;
	} else if ((new_value_int & 224) == 128) {
		port_type = 'd';
		port = (new_value_int & 127) >> 1;
		value = new_value_int & 1;
		x->first_byte = 0;
	} else if ((new_value_int & 128) == 0) {
		x->first_byte = new_value_int;
	}
	if (port_type == 'a' || port_type == 'd') {
		t_atom out[3];
		char out_data[2];
		out_data[2] = 0x00;
		sprintf(out_data, "%c", port_type);
		SETSYMBOL(&out[0], gensym(out_data));
		SETFLOAT(&out[1], port);
		SETFLOAT(&out[2], value);
        outlet_list(x->x_obj.ob_outlet, &s_list, 3, &out[0]);
	}	
}

void *arduino_message_parser_new(void) {
	t_arduino_message_parser *x = (t_arduino_message_parser *)pd_new(arduino_message_parser_class);
	outlet_new(&x->x_obj, &s_list);
	return (void *)x;
}

void arduino_message_parser_setup(void) {
	arduino_message_parser_class = class_new(gensym("arduino_message_parser"),
		(t_newmethod)arduino_message_parser_new,
		0, sizeof(t_arduino_message_parser), 0, 0);
	class_addfloat(arduino_message_parser_class, new_byte);
}

