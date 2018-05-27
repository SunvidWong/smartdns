/*************************************************************************
*
* Copyright (C) 2018 Ruilin Peng (Nick) <pymumu@gmail.com>.
*
 * smartdns is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * smartdns is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fast_ping.h"
#include "dns_client.h"
#include "dns_server.h"
#include "hashtable.h"
#include "list.h"
#include "tlog.h"
#include "atomic.h"
#include <stdio.h>
#include <stdlib.h>

atomic_t r = ATOMIC_INIT(0);

void print_result(struct ping_host_struct *ping_host, const char *host, FAST_PING_RESULT result, struct sockaddr *addr, socklen_t addr_len, int seqno,
							struct timeval *tv, void *userptr)
{
	atomic_inc(&r);
    #if 0
	if (result == PING_RESULT_RESPONSE) {
		double rtt = tv->tv_sec * 1000.0 + tv->tv_usec / 1000.0;
		printf("from %15s: seq=%d time=%.3f\n", host, seqno, rtt);
	} else if (result == PING_RESULT_TIMEOUT) {
		printf("from %15s: seq=%d timeout\n", host, seqno);
	}
    #endif
}

int smartdns_init()
{
    int ret;

	ret = tlog_init(".", "smartdns.log", 1024 * 1024, 8, 1, 0, 0);
    if (ret != 0) {
		fprintf(stderr, "start tlog failed.\n");
		goto errout;
	}

	tlog_setlogscreen(1);

	ret = fast_ping_init();
    if (ret != 0) {
        fprintf(stderr, "start ping failed.\n");
        goto errout;
    }

    ret = dns_server_init();
    if (ret != 0) {
        fprintf(stderr, "start dns server failed.\n");
        goto errout;
    }

    ret = dns_client_init();
    if (ret != 0) {
        fprintf(stderr, "start dns client failed.\n");
        goto errout;
    }

	dns_add_server("192.168.1.1", 53, DNS_SERVER_UDP);
    dns_add_server("114.114.114.114", 53, DNS_SERVER_UDP);
	dns_add_server("123.207.137.88", 53, DNS_SERVER_UDP);
	dns_add_server("193.112.15.186", 53, DNS_SERVER_UDP);
	dns_add_server("202.141.178.13", 5353, DNS_SERVER_UDP);
    dns_add_server("208.67.222.222", 5353, DNS_SERVER_UDP);
	dns_add_server("77.88.8.8", 53, DNS_SERVER_UDP);
	dns_add_server("202.141.162.123", 53, DNS_SERVER_UDP);
	dns_add_server("101.132.183.99", 53, DNS_SERVER_UDP);

	// int i = 0;
	// for(i = 0; i < 10; i++)
    // {
	// 	fast_ping_start("205.185.208.142", 1, 1000, print_result, NULL);
    //     fast_ping_start("205.185.208.142", 1, 1000, print_result, NULL);
    //     fast_ping_start("205.185.208.142", 1, 1000, print_result, NULL);
    //     fast_ping_start("205.185.208.142", 1, 1000, print_result, NULL);
    //     fast_ping_start("192.168.1.1", 1, 1000, print_result, NULL);
    //     fast_ping_start("192.168.1.1", 1, 1000, print_result, NULL);
    //     fast_ping_start("192.168.1.1", 1, 1000, print_result, NULL);
    //     fast_ping_start("192.168.1.1", 1, 1000, print_result, NULL);
    //     fast_ping_start("123.207.137.88", 1, 1000, print_result, NULL);
    //     fast_ping_start("123.207.137.88", 1, 1000, print_result, NULL);
    //     fast_ping_start("123.207.137.88", 1, 1000, print_result, NULL);
    //     fast_ping_start("123.207.137.88", 1, 1000, print_result, NULL);
	// }

	// sleep(2);
	// printf("i = %d, n = %d\n", i, atomic_read(&r));

    fast_ping_start("192.168.1.1", 10, 1000, NULL, NULL);

	return 0;
errout:

    return -1;
}

int smartdns_run()
{
    return dns_server_run();
}

void smartdns_exit()
{
    fast_ping_exit();
    dns_client_exit();
    dns_server_exit();
	tlog_exit();
}

struct data {
    struct list_head list;
    int n;
};

void list_test()
{
    struct list_head head;
    struct list_head *iter;
    int i = 0;

    INIT_LIST_HEAD(&head);

    for (i = 0; i < 10; i++) {
        struct data *h = malloc(sizeof(struct data));
        h->n = i;
        list_add(&h->list, &head);
    }

    list_for_each(iter, &head)
    {
        struct data *d = list_entry(iter, struct data, list);
        printf("%d\n", d->n);
    }
}

struct data_hash {
    struct hlist_node node;
    int n;
    char str[32];
};

int hash_test()
{
    DEFINE_HASHTABLE(ht, 7);
    struct data_hash *temp;
    struct data_hash *obj;
    int i;
    int key;

    for (i = 11; i < 17; i++) {
        temp = malloc(sizeof(struct data_hash));
        temp->n = i * i;
        hash_add(ht, &temp->node, temp->n);
    }

    for (i = 11; i < 17; i++) {
        key = i * i;
        hash_for_each_possible(ht, obj, node, key)
        {
            printf("value: %d\n", obj->n);
        };
    }

    return 0;
}

int hash_string_test()
{
    DEFINE_HASHTABLE(ht, 7);
    struct data_hash *temp;
    struct data_hash *obj;
    int i;
    int key;

    for (i = 0; i < 10; i++) {
        temp = malloc(sizeof(struct data_hash));
        sprintf(temp->str, "%d", i);
        hash_add(ht, &temp->node, hash_string(temp->str));
    }

    for (i = 0; i < 10; i++) {
        char key_str[32];
        sprintf(key_str, "%d", i);
        key = hash_string(key_str);
        hash_for_each_possible(ht, obj, node, key)
        {
            printf("i = %d value: %s\n", i, obj->str);
        };
    }

    return 0;
}

#if 0
struct data_rbtree {
	struct rb_node list;
	int value;
};

int rbtree_test()
{
	struct rb_root root;
	struct rb_node *n;
	RB_EMPTY_ROOT(&root);
	int i;

	for (i = 0; i < 10; i++)
	{
		struct data_rbtree *r = malloc(sizeof(struct data_rbtree));
		r->value = i;
		rb_insert(&r->list, &root);
	}

	n = rb_first(&root);
	int num = 5;
	while (n) {
		struct data_rbtree *r = container_of(n, struct data_rbtree, list);
		if (r->value < num) {
			n = n->rb_left;
		} else if (r->value > num) {
			n = n->rb_right;
		} else {
			printf("n = %d\n", r->value);
			break;
		}
	}

	return 0;
}
#endif

#include <signal.h>

void sig_handle(int sig)
{
	tlog(TLOG_ERROR, "process exit.\n");
	sleep(1);
	_exit(0);
}
int main(int argc, char *argv[])
{
    int ret;

    atexit(smartdns_exit);
	signal(SIGABRT, sig_handle);
	ret = smartdns_init();
    if (ret != 0) {
        fprintf(stderr, "init smartdns failed.\n");
        goto errout;
    }

    return smartdns_run();

errout:

    return 1;
}