/*
 * Action management functions.
 *
 * Copyright 2017 HAProxy Technologies, Christopher Faulet <cfaulet@haproxy.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <common/config.h>
#include <common/memory.h>
#include <common/mini-clist.h>
#include <common/standard.h>

#include <proto/action.h>
#include <proto/log.h>
#include <proto/proxy.h>
#include <proto/stick_table.h>


/* Check an action ruleset validity. It returns the number of error encountered
 * andd err_code is updated if a warning is emitted.
 */
int check_action_rules(struct list *rules, struct proxy *px, int *err_code)
{
	struct act_rule *rule;
	char *errmsg = NULL;
	int err = 0;

	list_for_each_entry(rule, rules, list) {
		if (rule->check_ptr && !rule->check_ptr(rule, px, &errmsg)) {
			ha_alert("Proxy '%s': %s.\n", px->id, errmsg);
			err++;
		}

		free(errmsg);
		errmsg = NULL;
	}

	return err;
}

/* Find and check the target table used by an action ACT_ACTION_TRK_*. This
 * function should be called during the configuration validity check.
 *
 * The function returns 1 in success case, otherwise, it returns 0 and err is
 * filled.
 */
int check_trk_action(struct act_rule *rule, struct proxy *px, char **err)
{
	struct proxy *target;

	if (rule->arg.trk_ctr.table.n)
		target = proxy_tbl_by_name(rule->arg.trk_ctr.table.n);
	else
		target = px;

	if (!target) {
		memprintf(err, "unable to find table '%s' referenced by track-sc%d",
			  rule->arg.trk_ctr.table.n, trk_idx(rule->action));
		return 0;
	}
	else if (target->table.size == 0) {
		memprintf(err, "table '%s' used but not configured",
			  rule->arg.trk_ctr.table.n ? rule->arg.trk_ctr.table.n : px->id);
		return 0;
	}
	else if (!stktable_compatible_sample(rule->arg.trk_ctr.expr,  target->table.type)) {
		memprintf(err, "stick-table '%s' uses a type incompatible with the 'track-sc%d' rule",
			  rule->arg.trk_ctr.table.n ? rule->arg.trk_ctr.table.n : px->id,
			  trk_idx(rule->action));
		return 0;
	}
	else if (px->bind_proc & ~target->bind_proc) {
		memprintf(err, "stick-table '%s' referenced by 'track-sc%d' rule not present on all processes covered by proxy '%s'",
			  target->id, trk_idx(rule->action), px->id);
		return 0;
	}
	else {
		free(rule->arg.trk_ctr.table.n);
		rule->arg.trk_ctr.table.t = &target->table;
		/* Note: if we decide to enhance the track-sc syntax, we may be
		 * able to pass a list of counters to track and allocate them
		 * right here using stktable_alloc_data_type().
		 */
	}
	return 1;
}

