/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Gpredict: Real-time satellite tracking and orbit prediction program

  Copyright (C)  2001-2009  Alexandru Csete, OZ9AEC.

  Authors: Alexandru Csete <oz9aec@gmail.com>

  Comments, questions and bugreports should be submitted via
  http://sourceforge.net/projects/gpredict/
  More details can be found at the project home page:

  http://gpredict.oz9aec.net/
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, visit http://www.fsf.org/
*/
/** \brief FREQ control.
 *
 * More info...
 * 
 *      1 222.333 444 MHz
 * 
 * \bug This should be a generic widget, not just frequency specific
 * 
 */
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <math.h>
#include "gtk-freq-knob.h"
#ifdef HAVE_CONFIG_H
#  include <build-config.h>
#endif



static void gtk_freq_knob_class_init (GtkFreqKnobClass *class);
static void gtk_freq_knob_init       (GtkFreqKnob      *list);
static void gtk_freq_knob_destroy    (GtkObject       *object);
static void gtk_freq_knob_update     (GtkFreqKnob *knob);
static void button_clicked_cb        (GtkWidget *button, gpointer data);

static GtkHBoxClass *parent_class = NULL;

#define FMTSTR "<span size='xx-large'>%c</span>"

/* x-index in table for buttons and labels */
static const guint idx[] = {
    0,
    2,
    3,
    4,
    6,
    7,
    8,
    10,
    11,
    12
};

static guint freq_changed_signal = 0;


GType
gtk_freq_knob_get_type ()
{
     static GType gtk_freq_knob_type = 0;

     if (!gtk_freq_knob_type) {

          static const GTypeInfo gtk_freq_knob_info = {
               sizeof (GtkFreqKnobClass),
               NULL,  /* base_init */
               NULL,  /* base_finalize */
               (GClassInitFunc) gtk_freq_knob_class_init,
               NULL,  /* class_finalize */
               NULL,  /* class_data */
               sizeof (GtkFreqKnob),
               5,     /* n_preallocs */
               (GInstanceInitFunc) gtk_freq_knob_init,
          };

          gtk_freq_knob_type = g_type_register_static (GTK_TYPE_VBOX,
                                                                "GtkFreqKnob",
                                                                 &gtk_freq_knob_info,
                                                                 0);
     }

     return gtk_freq_knob_type;
}


static void
gtk_freq_knob_class_init (GtkFreqKnobClass *class)
{
     GObjectClass      *gobject_class;
     GtkObjectClass    *object_class;
     GtkWidgetClass    *widget_class;
     GtkContainerClass *container_class;

     gobject_class   = G_OBJECT_CLASS (class);
     object_class    = (GtkObjectClass*) class;
     widget_class    = (GtkWidgetClass*) class;
     container_class = (GtkContainerClass*) class;

     parent_class = g_type_class_peek_parent (class);

     object_class->destroy = gtk_freq_knob_destroy;
    
    /* create freq changed signal */
    freq_changed_signal = g_signal_new ("freq-changed",
                                         G_TYPE_FROM_CLASS (class),
                                         G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                                         0, //G_STRUCT_OFFSET (GtkFreqKnobClass, tictactoe),
                                         NULL,
                                         NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);

 
}



static void
gtk_freq_knob_init (GtkFreqKnob *knob)
{
    knob->min = 0.0;
    knob->max = 9999999999.0;
    
}

static void
gtk_freq_knob_destroy (GtkObject *object)
{
     (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}



/** \brief Create a new Frequency control widget.
 * \param[in] val The initial value of the control.
 * \param[in] buttons Flag indicating whether buttons should be shown
 * \return A new frequency control widget.
 * 
 */
GtkWidget *
gtk_freq_knob_new (gdouble val, gboolean buttons)
{
    GtkWidget *widget;
    GtkWidget *table;
    GtkWidget *label;
    guint      i;
    gint       delta;


     widget = g_object_new (GTK_TYPE_FREQ_KNOB, NULL);

    GTK_FREQ_KNOB(widget)->value = val;
    
    table = gtk_table_new (3, 14, FALSE);
    
    /* create buttons and labels */
    for (i = 0; i < 10; i++) {
        /* labels */
        GTK_FREQ_KNOB(widget)->digits[i] = gtk_label_new (NULL);
        gtk_table_attach (GTK_TABLE (table), GTK_FREQ_KNOB(widget)->digits[i],
                          idx[i], idx[i]+1, 1, 2, GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0, 0);
    
        if (buttons) {
            /* UP buttons */
            GTK_FREQ_KNOB(widget)->buttons[i] = gtk_button_new ();
            
            label = gtk_label_new ("\342\226\264");
            gtk_container_add (GTK_CONTAINER(GTK_FREQ_KNOB(widget)->buttons[i]),
                            label);
            gtk_button_set_relief (GTK_BUTTON(GTK_FREQ_KNOB(widget)->buttons[i]),
                                GTK_RELIEF_NONE);
            delta = (gint) pow(10,9-i);
            g_object_set_data (G_OBJECT (GTK_FREQ_KNOB(widget)->buttons[i]),
                            "delta", GINT_TO_POINTER(delta)); 
            gtk_table_attach (GTK_TABLE (table), GTK_FREQ_KNOB(widget)->buttons[i],
                            idx[i], idx[i]+1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
            g_signal_connect (GTK_FREQ_KNOB(widget)->buttons[i], "clicked",
                            G_CALLBACK (button_clicked_cb), widget);
            
            /* DOWN buttons */
            GTK_FREQ_KNOB(widget)->buttons[i+10] = gtk_button_new ();
            
            label = gtk_label_new ("\342\226\276");
            gtk_container_add (GTK_CONTAINER(GTK_FREQ_KNOB(widget)->buttons[i+10]),
                            label);
            gtk_button_set_relief (GTK_BUTTON(GTK_FREQ_KNOB(widget)->buttons[i+10]),
                                GTK_RELIEF_NONE);
            g_object_set_data (G_OBJECT (GTK_FREQ_KNOB(widget)->buttons[i+10]),
                            "delta", GINT_TO_POINTER(-delta));
            gtk_table_attach (GTK_TABLE (table), GTK_FREQ_KNOB(widget)->buttons[i+10],
                            idx[i], idx[i]+1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
            g_signal_connect (GTK_FREQ_KNOB(widget)->buttons[i+10], "clicked",
                            G_CALLBACK (button_clicked_cb), widget);
        }
    }
    
    /* Add misc labels */
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<span size='xx-large'>.</span>");
    gtk_table_attach (GTK_TABLE (table), label, 5, 6, 1, 2,
                      GTK_SHRINK, GTK_SHRINK, 0, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<span size='xx-large'>.</span>");
    gtk_table_attach (GTK_TABLE (table), label, 9, 10, 1, 2,
                      GTK_SHRINK, GTK_SHRINK, 0, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<span size='xx-large'> Hz</span>");
    gtk_table_attach (GTK_TABLE (table), label, 13, 14, 1, 2,
                      GTK_SHRINK, GTK_SHRINK, 0, 0);
    
    
    gtk_freq_knob_update (GTK_FREQ_KNOB(widget));

    gtk_container_add (GTK_CONTAINER (widget), table);
    gtk_widget_show_all (widget);

     return widget;
}


/** \brief Set the value of the frequency control widget.
 * \param[in] knob THe frequency control widget.
 * \param[in] val The new value.
 * 
 */
void
gtk_freq_knob_set_value (GtkFreqKnob *knob, gdouble val)
{
    if ((val >= knob->min) && (val <= knob->max)) {
        /* set the new value */
        knob->value = val;
    
        /* update the display */
        gtk_freq_knob_update (knob);
    }
}


/** \brief Get the current value of the frequency control widget.
 *  \param[in] knob The frequency control widget.
 *  \return The current value.
 * 
 * Hint: For reading the value you can also access knob->value.
 * 
 */
gdouble
gtk_freq_knob_get_value (GtkFreqKnob *knob)
{
    return knob->value;
}



/** \brief Update frequency display widget.
 *  \param[in] knob The frequency control widget.
 * 
 */
static void
gtk_freq_knob_update     (GtkFreqKnob *knob)
{
    gchar b[11];
    gchar *buff;
    guint i;
    
    g_ascii_formatd (b, 11, "%10.0f", fabs(knob->value)); 
    
    /* set label markups */
    for (i = 0; i < 10; i++) {
        buff = g_strdup_printf (FMTSTR, b[i]);
        gtk_label_set_markup (GTK_LABEL(knob->digits[i]), buff);
        g_free (buff);
    }

}


/** \brief Button clicked event.
 * \param button The button that was clicked.
 * \param data Pointer to the GtkFreqKnob widget.
 * 
 */
static void
button_clicked_cb (GtkWidget *button, gpointer data)
{
    GtkFreqKnob *knob = GTK_FREQ_KNOB (data);
    gdouble delta = GPOINTER_TO_INT(g_object_get_data (G_OBJECT (button), "delta"));
    
    if ((delta > 0.0) && ((knob->value + delta) <= knob->max)) {
        knob->value += delta;
    }
    else if ((delta < 0.0) && ((knob->value + delta) >= knob->min)) {
        knob->value += delta;
    }
    
    gtk_freq_knob_update (knob);
    
    /* emit "freq_changed" signal */
    g_signal_emit (G_OBJECT (data), freq_changed_signal, 0);
}

