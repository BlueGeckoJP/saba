#include <dbus-1.0/dbus/dbus.h>
#include <stdio.h>
#include <string.h>

#include "metadata_utils.h"

int
get_playing_app(char* app_name, size_t len)
{
  DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, nullptr);
  if (!conn)
    return -1;

  DBusMessage* msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                                  "/org/freedesktop/DBus",
                                                  "org.freedesktop.DBus",
                                                  "ListNames");

  DBusMessage* reply =
    dbus_connection_send_with_reply_and_block(conn, msg, 1000, nullptr);
  dbus_message_unref(msg);

  if (!reply)
  {
    dbus_connection_unref(conn);
    return -1;
  }

  DBusMessageIter iter, array_iter;
  dbus_message_iter_init(reply, &iter);
  dbus_message_iter_recurse(&iter, &array_iter);

  while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING)
  {
    const char* name = nullptr;
    dbus_message_iter_get_basic(&array_iter, &name);

    if (strncmp(name, "org.mpris.MediaPlayer2.", 23) == 0)
    {
      DBusMessage* check_msg =
        dbus_message_new_method_call(name,
                                     "/org/mpris/MediaPlayer2",
                                     "org.freedesktop.DBus.Properties",
                                     "Get");

      const char* interface = "org.mpris.MediaPlayer2.Player";
      const char* property = "PlaybackStatus";

      dbus_message_append_args(check_msg,
                               DBUS_TYPE_STRING,
                               &interface,
                               DBUS_TYPE_STRING,
                               &property,
                               DBUS_TYPE_INVALID);

      DBusMessage* check_reply = dbus_connection_send_with_reply_and_block(
        conn, check_msg, 500, nullptr);
      dbus_message_unref(check_msg);

      if (check_reply)
      {
        DBusMessageIter check_iter, variant_iter;
        if (dbus_message_iter_init(check_reply, &check_iter))
        {
          if (dbus_message_iter_get_arg_type(&check_iter) == DBUS_TYPE_VARIANT)
          {
            dbus_message_iter_recurse(&check_iter, &variant_iter);

            if (dbus_message_iter_get_arg_type(&variant_iter) ==
                DBUS_TYPE_STRING)
            {
              const char* status = nullptr;
              dbus_message_iter_get_basic(&variant_iter, &status);

              if (strcmp(status, "Playing") == 0)
              {
                strlcpy(app_name, name + 23, len);
                dbus_message_unref(check_reply);
                dbus_message_unref(reply);
                dbus_connection_unref(conn);
                return 0;
              }
            }
          }
        }
        dbus_message_unref(check_reply);
      }
    }

    dbus_message_iter_next(&array_iter);
  }

  dbus_message_unref(reply);
  dbus_connection_unref(conn);
  return -1;
}

void
get_metadata(const char* app_name, TrackInfo* info)
{
  DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, nullptr);
  if (!conn)
    return;

  char service[256];
  snprintf(service, sizeof(service), "org.mpris.MediaPlayer2.%s", app_name);

  DBusMessage* msg =
    dbus_message_new_method_call(service,
                                 "/org/mpris/MediaPlayer2",
                                 "org.freedesktop.DBus.Properties",
                                 "Get");

  const char* interface = "org.mpris.MediaPlayer2.Player";
  const char* property = "Metadata";
  dbus_message_append_args(msg,
                           DBUS_TYPE_STRING,
                           &interface,
                           DBUS_TYPE_STRING,
                           &property,
                           DBUS_TYPE_INVALID);

  DBusMessage* reply =
    dbus_connection_send_with_reply_and_block(conn, msg, 1000, nullptr);
  dbus_message_unref(msg);

  if (!reply)
  {
    dbus_connection_unref(conn);
    return;
  }

  DBusMessageIter iter, variant_iter, array_iter, dict_iter;
  if (dbus_message_iter_init(reply, &iter))
  {
    if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT)
    {
      dbus_message_iter_recurse(&iter, &variant_iter);

      if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_ARRAY)
      {
        dbus_message_iter_recurse(&variant_iter, &array_iter);

        while (dbus_message_iter_get_arg_type(&array_iter) ==
               DBUS_TYPE_DICT_ENTRY)
        {
          dbus_message_iter_recurse(&array_iter, &dict_iter);

          char* key = nullptr;
          dbus_message_iter_get_basic(&dict_iter, &key);
          dbus_message_iter_next(&dict_iter);

          if (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_VARIANT)
          {
            DBusMessageIter value_iter;
            dbus_message_iter_recurse(&dict_iter, &value_iter);

            if (strcmp(key, "xesam:title") == 0 &&
                dbus_message_iter_get_arg_type(&value_iter) == DBUS_TYPE_STRING)
            {
              char* val = nullptr;
              dbus_message_iter_get_basic(&value_iter, &val);
              strlcpy(info->title, val, sizeof(info->title));
            }
            else if (strcmp(key, "xesam:artist") == 0 &&
                     dbus_message_iter_get_arg_type(&value_iter) ==
                       DBUS_TYPE_ARRAY)
            {
              DBusMessageIter artist_iter;
              dbus_message_iter_recurse(&value_iter, &artist_iter);

              if (dbus_message_iter_get_arg_type(&artist_iter) ==
                  DBUS_TYPE_STRING)
              {
                char* val = nullptr;
                dbus_message_iter_get_basic(&artist_iter, &val);
                strlcpy(info->artist, val, sizeof(info->artist));
              }
            }
            else if (strcmp(key, "mpris:length") == 0 &&
                     dbus_message_iter_get_arg_type(&value_iter) ==
                       DBUS_TYPE_INT64)
            {
              dbus_message_iter_get_basic(&value_iter, &info->duration_ms);
              info->duration_ms /= 1000;
            }
          }

          dbus_message_iter_next(&array_iter);
        }
      }
    }
  }

  dbus_message_unref(reply);

  msg = dbus_message_new_method_call(service,
                                     "/org/mpris/MediaPlayer2",
                                     "org.freedesktop.DBus.Properties",
                                     "Get");

  const char* property_position = "Position";
  dbus_message_append_args(msg,
                           DBUS_TYPE_STRING,
                           &interface,
                           DBUS_TYPE_STRING,
                           &property_position,
                           DBUS_TYPE_INVALID);

  reply = dbus_connection_send_with_reply_and_block(conn, msg, 1000, nullptr);
  dbus_message_unref(msg);

  if (reply)
  {
    if (dbus_message_iter_init(reply, &iter))
    {
      if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT)
      {
        dbus_message_iter_recurse(&iter, &variant_iter);
        if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_INT64)
        {
          dbus_message_iter_get_basic(&variant_iter, &info->position_ms);
          info->position_ms /= 1000;
        }
      }
    }
    dbus_message_unref(reply);
  }

  dbus_connection_unref(conn);
}