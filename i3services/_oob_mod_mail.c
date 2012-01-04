/*
 * oob_mod_mail.c
 *
 * Tricky@Rock the Halo
 * 16-MAY-2008
 * Intermud3 OOB mail service
 *
 * Needs coding.
 *
 */

#include <driver/runtime_config.h>
#include <intermud3.h>

#define TRUNCATED "***TRUNCATED***\n"

staticv object I3, Oob;
staticv mapping MailIDs;
staticv int MailID;

void create()
{
    MailIDs = ([ ]);
    MailID = 0;

    find_object(I3_CHDMOD)->add_service_name("mail", 1);

    if (!objectp(Oob)) Oob = find_object(I3_OOB);

    Oob->add_oob_service("mail", "mail_handler");
    Oob->add_oob_service("mail-ack", "mail_ack_handler");
}

void remove()
{
    find_object(I3_CHDMOD)->remove_service_name("mail");

    if (!objectp(Oob)) Oob = find_object(I3_OOB);

    Oob->remove_oob_service("mail");
    Oob->remove_oob_service("mail-ack");

    destruct();
}

void new_mail_id()
{
    MailID = max(MailID, time());

    while (MailIDs[MailID]) MailID++;
}

int send_mail(string t_mudname, string o_user, mapping to_list, mapping cc_list, string array bcc_list, string subject, string contents)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    string array to_mail_packet;
    string array cc_mail_packet;
    string array bcc_mail_packet;

    if (mudlist[t_mudname][0] != -1) return -1;
    if (member_array("mail", keys(mudlist[t_mudname][11])) == -1) return -2;

    new_mail_id();
    MailIDs[MailID] = ({ find_player(o_user), t_mudname, to_list, cc_list, bcc_list, subject });
    bcc_mail_packet = ({ "mail", MailID, o_user, to_list, cc_list, bcc_list, time(), subject, contents });

    new_mail_id();
    MailIDs[MailID] = ({ find_player(o_user), t_mudname, to_list, cc_list, bcc_list, subject });
    to_mail_packet = ({ "mail", MailID, o_user, to_list, cc_list, ({ }), time(), subject, contents });

    new_mail_id();
    MailIDs[MailID] = ({ find_player(o_user), t_mudname, to_list, cc_list, bcc_list, subject });
    cc_mail_packet = ({ "mail", MailID, o_user, to_list, cc_list, ({ }), time(), subject, contents });

    if (!objectp(Oob)) Oob = find_object(I3_OOB);

    if (sizeof(to_list) > 0)
        foreach (string mud in keys(to_list))
            Oob->send_oob_packet(mud, to_mail_packet, 1);

    if (sizeof(cc_list) > 0)
        foreach (string mud in keys(cc_list))
            Oob->send_oob_packet(mud, cc_mail_packet, 1);

    if (sizeof(bcc_list) > 0)
        Oob->send_oob_packet(t_mudname, bcc_mail_packet, 1);

    return 1;
}

void mail_handler(string o_mudname, mixed array packet)
{
    mapping ack_list = ([ ]);
    string array userlist = ({ });
    string array to_userlist = ({ });
    string array cc_userlist = ({ });
    string array not_here = ({ });

    if (sizeof(packet[5]) > 0)
    {
        userlist = filter(packet[5], (: user_exists($1) :) );
        not_here = packet[5] - userlist;
    }

    if (sizeof(packet[3]) > 1)
    {
        foreach (string mud, string array usrs in packet[3])
        {
            if (mud == mud_name())
            {
                string array tmp;

                tmp = filter(usrs, (: user_exists($1) :) );
                to_userlist = tmp;
                not_here += (usrs - tmp);
            }
        }
    }

    if (sizeof(packet[4]) > 1)
    {
        foreach (string mud, string array usrs in packet[4])
        {
            if (mud == mud_name())
            {
                string array tmp;

                tmp = filter(usrs, (: user_exists($1) :) );
                cc_userlist = tmp;
                not_here += (usrs - tmp);
            }
        }
    }

    /* Do the mailing here */

    ack_list[packet[1]] = map(not_here, (: ({ "'" + $1 + "' is not a user on " + $2 }) :), mud_name());

    if (!objectp(Oob)) Oob = find_object(I3_OOB);

    Oob->send_oob_ack_packet(o_mudname, ({ "mail-ack", ack_list }) );
}

void mail_ack_handler(string o_mudname, mixed array packet)
{
    foreach (int id, string array errors in packet[1])
    {
        if (MailIDs[id])
        {
            if (sizeof(errors) > 1)
            {
                /* Error report here */
            }

            map_delete(MailIDs, id);
        }
    }
}
