#include<stdlib.h>
#include<gtk/gtk.h>
#include<sndfile.h>
#include<stdio.h>
#include<limits.h>
#include<math.h>

#define TO_FLOAT(x) ((double)(x)/(double)SHRT_MAX)

#define SHOW_FRAMES 100 

static int show_frames = 100;

static GtkWidget *spin;
static GtkWidget *window_spin;
static GtkWidget *drawing_area;
static GtkAdjustment *scroll_adj;

static const float SCAN_DOMAIN_RATIO = 1.0f;
static const int MIN_FREQ = 30;
static const int MAX_FREQ = 2100;
static const int MAX_FALLS = 20;
static const int WINDOW_LENGTH = 500;
static const double EXP_FALL_MULT = 2.0;
//Constants
static const double E_TO_15TH = 3269017.372472111;
static const double LN_2 = 0.693147181;

double exp_fall_function(double val)
{
	return EXP_FALL_MULT * pow(M_E, log(val)/LN_2) / E_TO_15TH;
}

static short *snd_data;
static float *p_pos;

SF_INFO *file_info;
void delete(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

gboolean
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height;
	GdkRGBA color;
	
	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);
	gtk_style_context_get_color (gtk_widget_get_style_context (widget),
	                             0,
	                             &color);
	gdk_cairo_set_source_rgba (cr, &color);

	int draw_offset = (int) gtk_adjustment_get_value(scroll_adj);
	int frames = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(window_spin));
	double x = 0.0;
	double x_step = (double) width / (double) frames;

	cairo_move_to(cr, x, (height/2.0 - (TO_FLOAT(snd_data[draw_offset]) * height / 2.0 )));
	for(int i=1; i < frames; i++) {
		x += x_step;
		cairo_line_to(cr, x, (height/2.0 - (TO_FLOAT(snd_data[i+draw_offset]) * height / 2.0 )));
		if(i%200 == 0) {
			cairo_stroke(cr);
		}
	}
	cairo_stroke(cr);
	cairo_fill (cr);

	return FALSE;
}

void
drawing_area_redraw_cb(GtkWidget* widget, gpointer data)
{
	gtk_widget_queue_draw(drawing_area);
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	if(argc != 2) {
		exit(1);
	}
	
	file_info = malloc(sizeof(SF_INFO));
	SNDFILE *sndfile = sf_open (argv[1], SFM_READ, file_info);
	printf("channels: %d, samplerate: %d, samples: %lld\n", file_info->channels, file_info->samplerate, file_info->frames);
	snd_data = malloc(sizeof(short) * file_info->frames);
	float *snd_dataf = malloc(sizeof(float) * file_info->frames);
	sf_count_t items_read = sf_read_short(sndfile, snd_data, file_info->frames);
	printf("items read: %lld\n", items_read);
	sf_close(sndfile);

	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy",
	                 G_CALLBACK(delete), window);

	GtkWidget *grid = gtk_grid_new();

	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, 1000, 200);
	g_signal_connect (drawing_area, "draw", G_CALLBACK(draw_callback),
	                  NULL);

	scroll_adj = gtk_adjustment_new(0.0, 0.0, (double) file_info->frames, 1.0, SHOW_FRAMES, SHOW_FRAMES);
	g_signal_connect(G_OBJECT(scroll_adj), "value-changed", 
	                 G_CALLBACK(drawing_area_redraw_cb), NULL);

	GtkWidget *scrollbar = gtk_hscrollbar_new(GTK_ADJUSTMENT(scroll_adj));
	gtk_grid_attach(GTK_GRID(grid), scrollbar, 0, 1, 2, 1);

	spin = gtk_spin_button_new_with_range(0.0, (double) file_info->frames, 1.0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(spin), GTK_ADJUSTMENT(scroll_adj));
	gtk_grid_attach(GTK_GRID(grid), spin, 0, 2, 1, 1);
	g_signal_connect(G_OBJECT(spin), "value-changed", 
	                 G_CALLBACK(drawing_area_redraw_cb), NULL);

	window_spin = gtk_spin_button_new_with_range(1.0, (double) file_info->frames, 1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(window_spin), 1000.0);
	gtk_grid_attach(GTK_GRID(grid), window_spin, 1,2,1,1);
	g_signal_connect(G_OBJECT(window_spin), "value-changed", 
	                 G_CALLBACK(drawing_area_redraw_cb), NULL);
	
	gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 0, 2, 1);

	gtk_container_add(GTK_CONTAINER(window), grid);

	gtk_widget_show (grid);
	gtk_widget_show (drawing_area);
	gtk_widget_show_all (window);
	gtk_main();

	return 0;
}
