#if defined _serverredirect_included_
  #endinput
#endif
#define _serverredirect_included_

public SharedPlugin:__pl_serverredir = 
{
	name = "serverredir",
	file = "serverredirect.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};
 
public __pl_serverredir_SetNTVOptional()
{
	MarkNativeAsOptional("ShowServerRedirectMenu");
	MarkNativeAsOptional("LoadServerRedirectList");
	MarkNativeAsOptional("LoadServerRedirectListFiltered");
}

functag OnRedirectServersLoaded public(serverCount, const String:error[], Handle:serverList, any:data);
 
/**
 * Adds two numbers together.
 *
 * @param client	The client which will need to see the redirect menu.
 * @return		Returns nothing
 */
native ShowServerRedirectMenu(client);

/**
 * Loads the list of servers available for redirection. This call will use the
 * callback function to return the server list. The list will be filtered using the
 * sm_redirect_showoffline and sm_redirect_showcurrent convars.
 *
 * @param callback			Callback function, called as soon as the server list is loaded.
 * @param data				User data
 * @return					Returns nothing.
 */
native LoadServerRedirectList(OnRedirectServersLoaded:callback, any:data);

/**
 * Loads the list of servers available for redirection. This call will use the
 * callback function to return the server list. In contradiction to LoadServerRedirectList()
 * this function allows you to specify additional filter parameters.
 *
 * @param showCurrent		Whether to add the current server to the list
 * @param showOffline		Whether to add offline servers to the list
 * @param callback			Callback function, called as soon as the server list is loaded.
 * @param data				User data
 * @return					Returns nothing.
 */
native LoadServerRedirectListFiltered(bool:showCurrent, bool:showOffline, OnRedirectServersLoaded:callback, any:data);