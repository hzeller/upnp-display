
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <upnp/ixml.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char kMediaRendererDevice[] =
	"urn:schemas-upnp-org:device:MediaRenderer:1";

static const char kTransportService[] =
	"urn:schemas-upnp-org:service:AVTransport:1";
static const char kRenderControl[] =
	"urn:schemas-upnp-org:service:RenderingControl:1";


volatile int registered = 0;
UpnpClient_Handle device;

// get content (document, "foo/bar/baz")

static const char *get_node_content(IXML_Node *node) {
	IXML_Node *text_content = ixmlNode_getFirstChild(node);
	if (!text_content) return NULL;
	return ixmlNode_getNodeValue(text_content);
}

// Get the first node with the given name and return its content.
static const char *find_first_content(IXML_Node *node, const char *name) {
	IXML_NodeList *nlist = NULL;
	nlist = ixmlElement_getElementsByTagName((IXML_Element*)node, name);
	if (nlist == NULL) return NULL;
	if (nlist->nodeItem == NULL) return NULL;
	return get_node_content(nlist->nodeItem);
}

static const char *find_first_content_doc(IXML_Document *doc, const char *name) {
	IXML_NodeList *nlist = NULL;
	nlist = ixmlDocument_getElementsByTagName(doc, name);
	if (nlist == NULL) return NULL;
	if (nlist->nodeItem == NULL) return NULL;
	return get_node_content(nlist->nodeItem);
}

static int Register(const struct Upnp_Discovery *data) {
	int result = 0;
	int rc;
	// TODO: compare unique id if any.

	IXML_Document *service_description = NULL;
	rc = UpnpDownloadXmlDoc(data->Location, &service_description);
	if (rc != UPNP_E_SUCCESS) {
		fprintf(stderr, "Can't read service description\n");
		return result;
	}

	const char *base_url = find_first_content_doc(service_description,
						      "URLBase");
	if (base_url == NULL) {
		fprintf(stderr, "No URLBase defined\n");
		goto out;
	}

	const char *friendly_name = find_first_content_doc(service_description,
							   "friendlyName");

	// TODO: compare friendly name.
	if (strcmp(friendly_name, "hello") != 0)
		goto out;

	char *as_string = ixmlDocumenttoString(service_description);
	fprintf(stderr, "doc: %s\n", as_string);
	free(as_string);

	IXML_NodeList *service_list = NULL;
	service_list = ixmlDocument_getElementsByTagName(service_description,
							 "serviceList");

	if (service_list == NULL) {
		fprintf(stderr, "No services found for %s (%s)\n",
			friendly_name, data->DeviceId);
		goto out;
	}
	IXML_NodeList *service_it = NULL;
	service_it = ixmlElement_getElementsByTagName(
			(IXML_Element*) service_list->nodeItem,
			"service");
	if (service_it == NULL) {
		goto out;
	}

	for (/**/; service_it; service_it = service_it->next) {
		const char *service_type =
			find_first_content(service_it->nodeItem,
					   "serviceType");
		if (service_type == NULL) continue;
		const char *event_url =
			find_first_content(service_it->nodeItem,
					   "eventSubURL");

		if (strcmp(service_type, kTransportService) == 0) {
			char url[2 * LINE_SIZE];
			snprintf(url, sizeof(url), "%s%s", base_url, event_url + 1);
			fprintf(stderr, "subscribe: %s %s\n", service_type, url);
			int timeout;
			Upnp_SID sid;
			rc = UpnpSubscribe(device, url, &timeout, sid);
			fprintf(stderr, "Subscribe: %s %s %s rc=%d\n",
				friendly_name,
				service_type, UpnpGetErrorMessage(rc), rc);
		}
	}

	result = 1;
 out:
	ixmlDocument_free(service_description);
	return result;
}

static void upnp_discovery(Upnp_EventType event,
			   const struct Upnp_Discovery *data) {
	if (strcmp(data->DeviceType, kMediaRendererDevice) != 0)
		return;

	printf("DISCOVER: %s %s\n", data->DeviceId, data->DeviceType);


	switch (event) {
	case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		if (!registered) {
			registered = 1;   // keep track of uuid.
			if (!Register(data)) {
				registered = 0;
			}
		}
		break;

	case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		// TODO: compare that we only unregister same UUID
		if (registered) fprintf(stderr, "Unregister %s\n",
					data->DeviceType);
		registered = 0;
		break;

	case UPNP_EVENT_AUTORENEWAL_FAILED:
		fprintf(stderr, "Autorenewal failed\n");
		registered = 0;
		break;

	default:
		/* don't care */
		;
	}
}

static void upnp_receive_event(const struct Upnp_Event *data) {
	const char *as_string = find_first_content_doc(data->ChangedVariables,
						       "LastChange");
	fprintf(stderr, "Got variable changes: %s\n", as_string);
}

static int event_handler(Upnp_EventType event, void *event_data,
			 void *userdata) {
	switch (event) {
	case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
	case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
	case UPNP_EVENT_AUTORENEWAL_FAILED:
	case UPNP_DISCOVERY_SEARCH_RESULT:
		upnp_discovery(event, event_data);
		break;

	case UPNP_EVENT_RECEIVED:
		upnp_receive_event(event_data);
		break;

	default:
		fprintf(stderr, "don't care about event %d\n", event);
	}
	return UPNP_E_SUCCESS;
}

int main(int argc, char *argv[]) {
	UpnpInit(NULL, 0);
	UpnpRegisterClient(&event_handler, NULL, &device);
	getchar();
	return 0;
}
