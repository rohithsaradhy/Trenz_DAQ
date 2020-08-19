
// ---------------------------------------------------------------------
// list-gpio-lines.c
//
//   This is an example of GPIO access with the libGPIOd library.
//
// Copyright 2018 Christophe BLAESS (christophe@blaess.fr)
// License GPL v2.
// ---------------------------------------------------------------------

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>

int list_gpio_lines(struct gpiod_chip *chip)
{
	struct gpiod_line_bulk bulk;
	struct gpiod_line *line;
	int offset;
	const char *string;

	fprintf(stdout, "%s - %s - %d lines\n",
		gpiod_chip_name(chip),
		gpiod_chip_label(chip),
		gpiod_chip_num_lines(chip));

	if (gpiod_chip_get_all_lines(chip, &bulk) != 0) {
		perror("gpiod_chip_get_all_lines()");
		return -1;
	}

	gpiod_line_bulk_foreach_line_off(&bulk, line, offset) {
		fprintf(stdout, "  %2d: ", offset);

		string = gpiod_line_name(line);
		if (string == NULL)
			fprintf(stdout, "- ");
		else
			fprintf(stdout, "%s ", string);

		string = gpiod_line_consumer(line);
		if (string == NULL)
			fprintf(stdout, "() ");
		else
			fprintf(stdout, "(%s) ", string);;

		if (gpiod_line_direction(line) == GPIOD_LINE_DIRECTION_OUTPUT)
			fprintf(stdout, "out ");
		else
			fprintf(stdout, "in  ");

		if (gpiod_line_active_state(line) == GPIOD_LINE_ACTIVE_STATE_LOW)
			fprintf(stdout, "active-low ");

		if (gpiod_line_is_open_drain(line))
			fprintf(stdout, "open-drain ");

		if (gpiod_line_is_open_source(line))
			fprintf(stdout, "open-source ");

		if (gpiod_line_is_used(line))
			fprintf(stdout, "* ");

		fprintf(stdout, "\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct gpiod_chip_iter *iter;
	struct gpiod_chip *chip;
	int i;

	if (argc == 1) {
		// No argument: iterate on every GPIO chip.
		iter = gpiod_chip_iter_new();
		if (iter == NULL) {
			perror("gpiod_chip_iter");
			exit(EXIT_FAILURE);
		}
		while ((chip = gpiod_chip_iter_next(iter)) != NULL)
			list_gpio_lines(chip);
		gpiod_chip_iter_free(iter);

	} else {
		for (i = 1; i < argc; i ++) {
			chip = gpiod_chip_open_lookup(argv[i]);
			if (chip == NULL)
				perror(argv[i]);
			else
				list_gpio_lines(chip);
		}
	}

	return EXIT_SUCCESS;
}
