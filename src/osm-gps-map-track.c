/* osm-gps-map-track.c */

#include <gdk/gdk.h>

#include "converter.h"
#include "osm-gps-map-track.h"

G_DEFINE_TYPE (OsmGpsMapTrack, osm_gps_map_track, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VISIBLE,
    PROP_TRACK,
    PROP_LINE_WIDTH,
    PROP_ALPHA,
    PROP_COLOR
};

enum
{
	POINT_ADDED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0,};

struct _OsmGpsMapTrackPrivate
{
    GSList *track;
    gboolean visible;
    gfloat linewidth;
    gfloat alpha;
    GdkColor color;
};

#define DEFAULT_R   (60000)
#define DEFAULT_G   (0)
#define DEFAULT_B   (0)
#define DEFAULT_A   (0.6)

static void
osm_gps_map_track_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            g_value_set_boolean(value, priv->visible);
            break;
        case PROP_TRACK:
            g_value_set_pointer(value, priv->track);
            break;
        case PROP_LINE_WIDTH:
            g_value_set_float(value, priv->linewidth);
            break;
        case PROP_ALPHA:
            g_value_set_float(value, priv->alpha);
            break;
        case PROP_COLOR:
            g_value_set_boxed(value, &priv->color);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_track_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            priv->visible = g_value_get_boolean (value);
            break;
        case PROP_TRACK:
            priv->track = g_value_get_pointer (value);
            break;
        case PROP_LINE_WIDTH:
            priv->linewidth = g_value_get_float (value);
            break;
        case PROP_ALPHA:
            priv->alpha = g_value_get_float (value);
            break;
        case PROP_COLOR: {
            GdkColor *c = g_value_get_boxed (value);
            priv->color.red = c->red;
            priv->color.green = c->green;
            priv->color.blue = c->blue;
            } break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_track_dispose (GObject *object)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (object));
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    if (priv->track) {
        g_slist_foreach(priv->track, (GFunc) g_free, NULL);
        g_slist_free(priv->track);
        priv->track = NULL;
    }

    G_OBJECT_CLASS (osm_gps_map_track_parent_class)->dispose (object);
}

static void
osm_gps_map_track_finalize (GObject *object)
{
    G_OBJECT_CLASS (osm_gps_map_track_parent_class)->finalize (object);
}

static void
osm_gps_map_track_class_init (OsmGpsMapTrackClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (OsmGpsMapTrackPrivate));

    object_class->get_property = osm_gps_map_track_get_property;
    object_class->set_property = osm_gps_map_track_set_property;
    object_class->dispose = osm_gps_map_track_dispose;
    object_class->finalize = osm_gps_map_track_finalize;

    g_object_class_install_property (object_class,
                                     PROP_VISIBLE,
                                     g_param_spec_boolean ("visible",
                                                           "visible",
                                                           "should this track be visible",
                                                           TRUE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_TRACK,
                                     g_param_spec_pointer ("track",
                                                           "track",
                                                           "list of points for the track",
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (object_class,
                                     PROP_LINE_WIDTH,
                                     g_param_spec_float ("line-width",
                                                         "line-width",
                                                         "width of the lines drawn for the track",
                                                         0.0,       /* minimum property value */
                                                         100.0,     /* maximum property value */
                                                         4.0,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_ALPHA,
                                     g_param_spec_float ("alpha",
                                                         "alpha",
                                                         "alpha transparency of the track",
                                                         0.0,       /* minimum property value */
                                                         1.0,       /* maximum property value */
                                                         DEFAULT_A,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_COLOR,
                                     g_param_spec_boxed ("color",
                                                         "color",
                                                         "color of the track",
                                                         GDK_TYPE_COLOR,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

	/**
	 * OsmGpsMapTrack::point-added:
	 * @self: A #OsmGpsMapTrack
	 *
	 * The point-added signal.
	 */
	signals [POINT_ADDED] = g_signal_new ("point-added",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__BOXED,
	                            G_TYPE_NONE,
	                            1,
                                OSM_TYPE_GPS_MAP_POINT);
}

static void
osm_gps_map_track_init (OsmGpsMapTrack *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE((self), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackPrivate);

    self->priv->color.red = DEFAULT_R;
    self->priv->color.green = DEFAULT_G;
    self->priv->color.blue = DEFAULT_B;
}

void
osm_gps_map_track_add_point (OsmGpsMapTrack *track, OsmGpsMapPoint *point)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (track));
    OsmGpsMapTrackPrivate *priv = track->priv;

    priv->track = g_slist_append (priv->track, point);
    g_signal_emit (track, signals[POINT_ADDED], 0, point);
}

GSList *
osm_gps_map_track_get_points (OsmGpsMapTrack *track)
{
    g_return_val_if_fail (OSM_IS_GPS_MAP_TRACK (track), NULL);
    return track->priv->track;
}

void
osm_gps_map_track_get_color (OsmGpsMapTrack *track, GdkColor *color, gfloat *alpha)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (track));
    color->red = track->priv->color.red;
    color->green = track->priv->color.green;
    color->blue = track->priv->color.blue;
    *alpha = track->priv->alpha;
}


OsmGpsMapTrack *
osm_gps_map_track_new (void)
{
    return g_object_new (OSM_TYPE_GPS_MAP_TRACK, NULL);
}

OsmGpsMapPoint *
osm_gps_map_point_new_degrees(gdouble lat, gdouble lon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = deg2rad(lat);
    p->rlon = deg2rad(lon);
    return p;
}

OsmGpsMapPoint *
osm_gps_map_point_new_radians(gdouble rlat, gdouble rlon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = rlat;
    p->rlon = rlon;
    return p;
}

void
osm_gps_map_point_as_degrees(OsmGpsMapPoint *point, gdouble *lat, gdouble *lon)
{
    *lat = rad2deg(point->rlat);
    *lon = rad2deg(point->rlon);
}

void
osm_gps_map_point_as_radians(OsmGpsMapPoint *point, gdouble *rlat, gdouble *rlon)
{
    *rlat = point->rlat;
    *rlon = point->rlon;
}

static OsmGpsMapPoint *
osm_gps_map_point_copy (const OsmGpsMapPoint *point)
{
    OsmGpsMapPoint *result = g_new (OsmGpsMapPoint, 1);
    *result = *point;

    return result;
}

GType
osm_gps_map_point_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type = g_boxed_type_register_static (g_intern_static_string ("OsmGpsMapPoint"),
				         (GBoxedCopyFunc)osm_gps_map_point_copy,
				         (GBoxedFreeFunc)g_free);
    return our_type;
}
