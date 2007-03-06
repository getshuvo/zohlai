/*
 * Copyright (c) 2005 William Pitcock, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the CService STATUS function.
 *
 * $Id: status.c 7855 2007-03-06 00:43:08Z pippijn $
 */

#include "atheme.h"

DECLARE_MODULE_V1
(
	"nickserv/status", FALSE, _modinit, _moddeinit,
	"$Id: status.c 7855 2007-03-06 00:43:08Z pippijn $",
	"Atheme Development Group <http://www.atheme.org>"
);

static void ns_cmd_acc(sourceinfo_t *si, int parc, char *parv[]);
static void ns_cmd_status(sourceinfo_t *si, int parc, char *parv[]);

command_t ns_status = { "STATUS", N_("Displays session information."), AC_NONE, 0, ns_cmd_status };
command_t ns_acc = { "ACC", N_("Displays parsable session information"), AC_NONE, 2, ns_cmd_acc };

list_t *ns_cmdtree, *ns_helptree;

void _modinit(module_t *m)
{
	MODULE_USE_SYMBOL(ns_cmdtree, "nickserv/main", "ns_cmdtree");
	MODULE_USE_SYMBOL(ns_helptree, "nickserv/main", "ns_helptree");

	command_add(&ns_acc, ns_cmdtree);
	help_addentry(ns_helptree, "ACC", "help/nickserv/acc", NULL);
	command_add(&ns_status, ns_cmdtree);
	help_addentry(ns_helptree, "STATUS", "help/nickserv/status", NULL);
}

void _moddeinit()
{
	command_delete(&ns_acc, ns_cmdtree);
	help_delentry(ns_helptree, "ACC");
	command_delete(&ns_status, ns_cmdtree);
	help_delentry(ns_helptree, "STATUS");
}

static void ns_cmd_acc(sourceinfo_t *si, int parc, char *parv[])
{
	char *targuser = parv[0];
	char *targaccount = parv[1];
	user_t *u;
	myuser_t *mu;
	mynick_t *mn;

	if (!targuser)
	{
		u = si->su;
		targuser = u != NULL ? u->nick : "?";
	}
	else
		u = user_find_named(targuser);

	if (!u)
	{
		command_fail(si, fault_nosuch_target, "%s%s%s ACC 0 (offline)", targuser, parc >= 2 ? " -> " : "", parc >= 2 ? targaccount : "");
		return;
	}

	if (!targaccount)
		targaccount = u->nick;
	if (!strcmp(targaccount, "*"))
		mu = u->myuser;
	else
		mu = myuser_find_ext(targaccount);

	if (!mu)
	{
		command_fail(si, fault_nosuch_target, "%s%s%s ACC 0 (not registered)", u->nick, parc >= 2 ? " -> " : "", parc >= 2 ? targaccount : "");
		return;
	}

	if (u->myuser == mu)
		command_success_nodata(si, "%s%s%s ACC 3", u->nick, parc >= 2 ? " -> " : "", parc >= 2 ? mu->name : "");
	else if ((mn = mynick_find(u->nick)) != NULL && mn->owner == mu &&
			myuser_access_verify(u, mu))
		command_success_nodata(si, "%s%s%s ACC 2", u->nick, parc >= 2 ? " -> " : "", parc >= 2 ? mu->name : "");
	else
		command_success_nodata(si, "%s%s%s ACC 1", u->nick, parc >= 2 ? " -> " : "", parc >= 2 ? mu->name : "");
}

static void ns_cmd_status(sourceinfo_t *si, int parc, char *parv[])
{
	logcommand(si, CMDLOG_GET, "STATUS");

	if (!si->smu)
		command_success_nodata(si, "You are not logged in.");
	else
	{
		command_success_nodata(si, "You are logged in as \2%s\2.", si->smu->name);

		if (is_soper(si->smu))
		{
			soper_t *soper = si->smu->soper;

			command_success_nodata(si, "You are a services operator of class %s.", soper->operclass ? soper->operclass->name : soper->classname);
		}
	}

	if (si->su != NULL)
	{
		mynick_t *mn;

		mn = mynick_find(si->su->nick);
		if (mn != NULL && mn->owner != si->smu &&
				myuser_access_verify(si->su, mn->owner))
			command_success_nodata(si, "You are recognized as \2%s\2.", mn->owner->name);
	}

	if (si->su != NULL && is_admin(si->su))
		command_success_nodata(si, "You are a server administrator.");

	if (si->su != NULL && is_ircop(si->su))
		command_success_nodata(si, "You are an IRC operator.");
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
