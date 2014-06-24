/*------------------------------------------------------------------------\
|  net_link.c : Data Structures                       www.middle-earth.us |
|  Copyright (C) 2005, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/resource.h>


#include "protos.h"
#include "account.h"
#include "net_link.h"
#include "utils.h"

/* global descriptor list */
DESCRIPTOR_DATA *descriptor_list = NULL;
DESCRIPTOR_DATA *next_to_process = NULL;

int maxdesc, avail_descs;
bool socket_closed = false;
int connected = 0;
int nTotalConnections = 0;

/* Private Interface */
int get_from_q (struct txt_q *queue, char *dest);


int
new_descriptor (int s)
{
    int desc;
    struct sockaddr_in sock;
    DESCRIPTOR_DATA *newd, *td;
    char ip[MAX_STRING_LENGTH];
    struct hostent *from = NULL;

    socklen_t size;

    size = sizeof (sock);

    if ((desc = accept (s, (struct sockaddr *) &sock, &size)) < 0)
    {
        perror ("Accept");
        return (-1);
    }

    nonblock (desc);
    if ((maxdesc + 1) >= avail_descs)
    {
        write (desc, "The game is full.  Try later.\n\r", 31);
        close (desc);
        return (0);
    }
    else if (desc > maxdesc)
        maxdesc = desc;

    newd = new struct descriptor_data;

    init_descriptor (newd, desc);

    *ip = '\0';
    strcpy (ip, inet_ntoa (sock.sin_addr));

    newd->strClientIpAddr = str_dup (ip);
    newd->strClientHostname = str_dup (ip);

    std::string resolved_hostname = resolved_host (newd->strClientHostname);
    if (resolved_hostname.empty ())
    {
        from = gethostbyaddr ((char *) &sock.sin_addr,
                              sizeof ((char *) & sock.sin_addr),
                              AF_INET);
        if (from && *from->h_name != '-')
        {
            mysql_safe_query ("INSERT INTO resolved_hosts "
                              "VALUES ('%s', '%s', %d)",
                              ip, from->h_name, (int) time (0));
            mem_free (newd->strClientHostname);
            newd->strClientHostname = str_dup (from->h_name);
        }
        else
        {
            mysql_safe_query
            ("INSERT INTO resolved_hosts VALUES ('%s', '%s', %d)", ip, ip,
             (int) time (0));
        }
    }
    else
    {
        mem_free (newd->strClientHostname);
        newd->strClientHostname = str_dup (resolved_hostname.c_str ());
        newd->resolved = 1;
    }

    if (!descriptor_list)
        descriptor_list = newd;
    else
        for (td = descriptor_list; td; td = td->next)
        {
            if (!td->next)
            {
                newd->next = NULL;
                td->next = newd;
                break;
            }
        }

    if (connected > MAX_CONNECTIONS)
    {
        SEND_TO_Q ("\r\n"
                   "We apologize for the inconvenience, but the MUD is currently full.\r\n"
                   "\r\n"
                   "Please try back again later. Thank you.\r\n" "\r\n", newd);
        newd->connected = CON_PENDING_DISC;
        return (0);
    }

    if (!maintenance_lock)
        SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), newd);
    else
        SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
                   newd);
    SEND_TO_Q ("Your Selection: ", newd);
    newd->connected = CON_LOGIN;

    return (0);
}

void
init_descriptor (DESCRIPTOR_DATA* newd, int desc)
{
    /* init desc data */
    nTotalConnections++;

    newd->hSocketFD = desc;
    newd->strClientHostname = 0;
    newd->strClientIpAddr = 0;
    newd->connected = CON_PLYNG;

    newd->wait = 1;
    newd->showstr_head = 0;
    newd->showstr_point = 0;
    newd->header = 0;
    newd->str = 0;

    newd->edit_type = 0;
    newd->edit_index = -1;
    newd->edit_string = 0;
    newd->edit_length = 0;
    newd->edit_line_first = 0;
    newd->max_str = 0;

    newd->prompt_mode = 1;
    newd->buf[0] = '\0';
    newd->last_input[0] = '\0';

    newd->output.head = 0;
    newd->output.tail = 0;
    newd->input.head = 0;
    newd->input.tail = 0;

    newd->acct = 0;
    newd->character = 0;
    newd->original = 0;
    newd->snoop.snooping = 0;
    newd->snoop.snoop_by = 0;
    newd->next = 0;
    newd->proc = 0;

    newd->lines = 0;
    newd->col = 0;
    newd->line = 0;
    newd->tos_line = 0;
    newd->max_lines = 0;
    newd->max_columns = 0;
    newd->edit_mode = 0 | MODE_DONE_EDITING;

    newd->login_time = time_now; /// \todo carry over from preboot time
    newd->pending_message = 0;
    newd->time_last_activity = mud_time;

    newd->idle = 0;
    newd->stored = 0;
    newd->color = 0;
    newd->resolved = 0;
}

void
free_descriptor (DESCRIPTOR_DATA * d)
{
    if (d->strClientHostname && *d->strClientHostname)
    {
        mem_free (d->strClientHostname);
    }

    if (d->strClientIpAddr && *d->strClientIpAddr)
    {
        mem_free (d->strClientIpAddr);
    }

    if (d->showstr_head && *d->showstr_head)
    {
        mem_free (d->showstr_head);
    }

    if (d->header && *d->header)
    {
        mem_free (d->header);
    }

    if (d->edit_string && *d->edit_string)
    {
        mem_free (d->edit_string);
    }

    delete d;
    d = 0;
}


/* Empty the queues before closing connection */
void
flush_queues (DESCRIPTOR_DATA * d)
{
    char dummy[MAX_STRING_LENGTH];

    while (get_from_q (&d->output, dummy));
    while (get_from_q (&d->input, dummy));
}

/* ******************************************************************
*  socket handling							 *
****************************************************************** */
int
init_socket (int port)
{
    int s, opt;
    struct sockaddr_in sa;
    struct linger ld;

    memset (&sa, 0, sizeof (struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);

    if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("init_socket: socket()");
        exit (1);
    }
    opt = 1;
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof (opt)) <
            0)
    {
        perror ("init_socket:setsockopt()");
        exit (1);
    }
    ld.l_onoff = 0;		/* let's make sure this isn't on */
    ld.l_linger = 1000;		/* who cares what this is */

    if (setsockopt (s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof (ld)) < 0)
    {
        perror ("setsockopt");
        exit (1);
    }

    if (bind (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {

        perror ("bind()");

        if (close (s))
        {
            perror ("error closing socket");
            exit (1);
        }

        exit (0);
    }

    if (listen (s, 5) < 0)
    {
        perror ("listen");
        exit (1);
    }
    return (s);
}

void
close_sockets (int s)
{
    system_log ("Closing all sockets.", false);

    while (descriptor_list)
        close_socket (descriptor_list);

    close (s);
}

void
close_socket (DESCRIPTOR_DATA * d)
{
    DESCRIPTOR_DATA *tmp;
    char buf[100];

    if (d->connected == CON_SOFT_REBOOT)	/* Soft reboot sockets. */
        return;

    if (d->character)		/* I don't know about this one. . . */
        d->character->descriptor = 0;

    close (d->hSocketFD);
    flush_queues (d);

    if (d->hSocketFD == maxdesc)
        --maxdesc;

    if (d->character && d->connected != CON_PLYNG)
    {
        unload_pc (d->character);
        d->character = NULL;
    }
    else if (d->character && d->connected == CON_PLYNG)
    {

        if (d->character->pc)
            d->character->pc->last_disconnect = time (0);

        save_char (d->character, true);

        /* KLUDGE:  If the player is disconnecting is staff, he will
           get a message about himself disconnecting.  However, he will
           be gone before that message gets to him, and the message
           will sit around in memory.  By saying he isn't connected, the
           message will not be sent.  (connected = -1)
         */

        if (d->original)
        {
            d->character = d->original;
            d->original = NULL;
        }

        sprintf (s_buf, "%s has lost link.\n", d->character->tname);
        d->connected = CON_LINKDEAD;
        send_to_gods (s_buf);
        d->connected = CON_PLYNG;

        if (d->snoop.snooping && d->snoop.snooping->descr())
        {
            d->snoop.snooping->descr()->snoop.snoop_by = 0;
            d->snoop.snooping = 0;
        }

        if (d->character->pc)
            d->character->pc->owner = 0;

        if (IS_MORTAL (d->character) && !IS_SET (d->character->flags, FLAG_GUEST))
        {
            act ("$n has lost link.", true, d->character, 0, 0, TO_ROOM);

            sprintf (buf, "Closing link to: %s.", GET_NAME (d->character));
            system_log (buf, false);

            d->character->descriptor = 0;
        }
        else if (IS_SET (d->character->flags, FLAG_GUEST))
        {
            do_quit (d->character, "", 0);
        }
    }

    if (d->acct)
        delete d->acct;

    if (next_to_process == d)
        next_to_process = next_to_process->next;

    if (d == descriptor_list)
    {
        /* this is the head of the list */
        descriptor_list = descriptor_list->next;
    }
    else
    {
        for (tmp = descriptor_list; tmp->next; tmp = tmp->next)
            if (tmp->next == d)
            {
                tmp->next = tmp->next->next;
                break;
            }
    }

    d->next = NULL;

    free_descriptor (d);

    socket_closed = true;
}

/* ******************************************************************
  general utility stuff (for local use)
****************************************************************** */

int
get_from_q (struct txt_q *queue, char *dest)
{
    struct txt_block *tmp;

    /* Q empty? */
    if (!queue->head)
        return (0);

    tmp = queue->head;
    strcpy (dest, queue->head->text);
    queue->head = queue->head->next;

    if (queue->head == NULL)
        queue->tail = NULL;

    mem_free (tmp->text); // struct txt_block
    mem_free (tmp); // struct txt_block

    return (1);
}

bool
get_from_q (struct txt_q *queue, std::string& dest)
{
    struct txt_block *tmp;

    /* Q empty? */
    if (!queue->head)
        return (false);

    tmp = queue->head;
    dest.assign (queue->head->text);
    queue->head = queue->head->next;

    if (queue->head == NULL)
        queue->tail = NULL;

    mem_free (tmp->text); // struct txt_block
    mem_free (tmp); // struct txt_block

    return (true);
}


struct timeval
            timediff (struct timeval *a, struct timeval *b)
{
    struct timeval rslt, tmp;
    extern int run_mult;		// Defined in comm.c
    tmp = *a;

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
    {
        rslt.tv_usec += 100 * run_mult;
        --(tmp.tv_sec);
    }
    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
    {

        rslt.tv_usec = 0;
        rslt.tv_sec = 0;
    }
    return (rslt);
}


int
process_output (DESCRIPTOR_DATA * t)
{
    char i[MAX_STRING_LENGTH + 1];

    if (!t->prompt_mode && !t->connected && t->edit_index == -1)
        if (write_to_descriptor (t, "\r\n") < 0)
            return (-1);

    /* Cycle thru output queue */
    while (get_from_q (&t->output, i))
    {
        if (t->snoop.snoop_by && t->snoop.snoop_by->descr() != NULL
                && !IS_NPC (t->snoop.snoop_by))
        {
            write_to_q ("% ", &t->snoop.snoop_by->descr()->output);
            write_to_q (i, &t->snoop.snoop_by->descr()->output);
        }

        if (write_to_descriptor (t, i))
            return (-1);
    }

    if (!t->connected && !(t->character && !IS_NPC (t->character) && GET_FLAG (t->character, FLAG_COMPACT)))
    {
        if (IS_SET (t->edit_mode, MODE_DONE_EDITING) && t->edit_index == -1)
        {
            if (write_to_descriptor (t, "\r\n") < 0)
            {
                return (-1);
            }
        }
    }

    return (1);
}

int
write_to_descriptor (DESCRIPTOR_DATA * d, const char *txt)
{
    int sofar;
    int thisround;
    int total;
    char ansi_buf[MAX_STRING_LENGTH * 2];

    colorize (txt, ansi_buf, d);

    total = strlen (ansi_buf);
    sofar = 0;

    do
    {
        thisround = write (d->hSocketFD, ansi_buf + sofar, total - sofar);
        if (thisround < 0)
        {
            perror ("Write to socket");
            return -1;
        }

        sofar += thisround;

    }
    while (sofar < total);

    return 0;
}

int
process_input (DESCRIPTOR_DATA * t)
{
    int sofar;
    int thisround;
    int begin;
    int squelch;
    int i;
    int k;
    int flag;
    char tmp[MAX_STRING_LENGTH + 100];
    char buffer[MAX_STRING_LENGTH + 100];

    sofar = 0;
    flag = 0;
    begin = strlen (t->buf);

    /* Read in some stuff */
    do
    {
        if ((thisround = read (t->hSocketFD, t->buf + begin + sofar,
                               MAX_INPUT_LENGTH - (begin + sofar) - 1)) > 0)
            sofar += thisround;
        else if (thisround < 0)
            if (errno != EWOULDBLOCK)
            {
                return -1;
            }
            else
                break;
        else
        {
            return -1;
        }
    }
    while (!ISNEWL (*(t->buf + begin + sofar - 1)));

    *(t->buf + begin + sofar) = 0;

    if (!IS_SET (t->edit_mode, MODE_DONE_EDITING))
    {
        ve_process (t, t->buf);	/* Editor subsystem call */
        *t->buf = '\0';		/* This may cause some data to be lost if */
        return (0);		/* chars are typed after @ & before processing */
    }

    /* if no newline is contained in input, return without proc'ing */
    for (i = begin; !ISNEWL (*(t->buf + i)); i++)
        if (!*(t->buf + i))
            return (0);

    /* input contains 1 or more newlines; process the stuff */
    for (i = 0, k = 0; *(t->buf + i);)
    {
        if (!ISNEWL (*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
            if (*(t->buf + i) == '\b')	/* backspace */
                if (k)		/* more than one char ? */
                {
                    i++;
                }
                else
                    i++;		/* no or just one char.. Skip backsp */
            else
                /* KILLER CDR:  $$ problem here. */
                if (isascii (*(t->buf + i)) && isprint (*(t->buf + i)))
                {
                    *(tmp + k) = *(t->buf + i);

                    k++;
                    i++;
                }
                else
                    i++;
        else
        {
            *(tmp + k) = 0;
            if (*tmp == '!')
                strcpy (tmp, t->last_input);
            else
                strcpy (t->last_input, tmp);

            write_to_q (tmp, &t->input);

            if (t->snoop.snoop_by && t->snoop.snoop_by->descr() != NULL
                    && !IS_NPC (t->snoop.snoop_by))
            {
                write_to_q ("% ", &t->snoop.snoop_by->descr()->output);
                write_to_q (tmp, &t->snoop.snoop_by->descr()->output);
                write_to_q ("\n\r", &t->snoop.snoop_by->descr()->output);
            }

            if (flag)
            {
                sprintf (buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
                if (write_to_descriptor (t, buffer) < 0)
                    return (-1);

                /* skip the rest of the line */
                for (; !ISNEWL (*(t->buf + i)); i++);
            }

            /* find end of entry */
            for (; ISNEWL (*(t->buf + i)); i++);

            /* squelch the entry from the buffer */
            for (squelch = 0;; squelch++)
                if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0')
                    break;
            k = 0;
            i = 0;
        }
    }

    return (1);
}

void
descriptor__drop_connections (fd_set * readfds, fd_set * writefds,
                              fd_set * exceptfds)
{
    DESCRIPTOR_DATA *ptr, *next;

    connected = 0;

    for (ptr = descriptor_list; ptr; ptr = next)
    {

        next = ptr->next;

        if (FD_ISSET (ptr->hSocketFD, exceptfds))
        {
            FD_CLR (ptr->hSocketFD, readfds);
            FD_CLR (ptr->hSocketFD, exceptfds);
            FD_CLR (ptr->hSocketFD, writefds);
            close_socket (ptr);
        }
        else
        {
            connected++;
        }
    }
}

void
nonblock (int s)
{
    if (fcntl (s, F_SETFL, FNDELAY) == -1)
    {
        perror ("Noblock");
        exit (1);
    }
}

void
write_to_q (const char *txt, struct txt_q *queue)
{
    struct txt_block *newPtr;
    int seen_cr = 0;
    int char_count = 0;
    const char *p;
    char *m;

    if (!queue)
        return;

    CREATE (newPtr, struct txt_block, 1);

    p = txt;

    while (*p)
    {

        if (*p == '\n')
        {

            if (!seen_cr)
            {
                seen_cr = 1;
                char_count++;
            }

            char_count++;
        }

        else if (*p != '\r')
        {
            char_count++;
            seen_cr = 0;
        }

        p++;
    }

    newPtr->text = (char *) alloc (char_count + 1, 2);

    p = txt;
    m = newPtr->text;
    seen_cr = 0;

    while (*p)
    {

        if (*p == '\n')
        {

            if (!seen_cr)
            {
                seen_cr = 1;
                *m = '\r';
                m++;
            }

            *m = '\n';
            m++;
        }

        else if (*p != '\r')
        {
            *m = *p;
            m++;
            seen_cr = 0;
        }

        p++;
    }

    *m = '\0';

    /* Q empty? */

    if (!queue->head)
    {
        newPtr->next = NULL;
        queue->head = queue->tail = newPtr;
    }

    else
    {
        queue->tail->next = newPtr;
        queue->tail = newPtr;
        newPtr->next = NULL;
    }
}
