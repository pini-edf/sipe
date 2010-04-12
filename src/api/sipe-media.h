/**
 * @file sipe-media.h
 *
 * pidgin-sipe
 *
 * Copyright (C) 2010 Jakub Adam <jakub.adam@tieto.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

struct sipe_account_data;

typedef enum sipe_media_type {
	SIPE_MEDIA_AUDIO,
	SIPE_MEDIA_VIDEO
} SipeMediaType;

typedef enum sipe_call_state {
	SIPE_CALL_CONNECTING,
	SIPE_CALL_RUNNING,
	SIPE_CALL_HELD,
	SIPE_CALL_FINISHED
} SipeCallState;

typedef enum sipe_component_type {
	SIPE_COMPONENT_NONE = 0,
	SIPE_COMPONENT_RTP  = 1,
	SIPE_COMPONENT_RTCP = 2
} SipeComponentType;

typedef enum sipe_candidate_type {
	SIPE_CANDIDATE_TYPE_HOST,
	SIPE_CANDIDATE_TYPE_RELAY,
	SIPE_CANDIDATE_TYPE_SRFLX
} SipeCandidateType;

typedef enum sipe_network_protocol {
	SIPE_NETWORK_PROTOCOL_TCP,
	SIPE_NETWORK_PROTOCOL_UDP
} SipeNetworkProtocol;

typedef gpointer sipe_codec;
typedef gpointer sipe_candidate;

typedef struct _sipe_media_call {
	gpointer			media;
	struct sip_session	*session;
	struct sip_dialog	*dialog;

	gchar				*remote_ip;
	guint16				remote_port;

	GSList				*sdp_attrs;
	struct sipmsg		*invitation;
	GList				*remote_candidates;
	GList				*remote_codecs;
	gchar				*sdp_response;
	gboolean			legacy_mode;
	SipeCallState		state;
} sipe_media_call;

void sipe_media_incoming_invite(struct sipe_account_data *sip, struct sipmsg *msg);

void sipe_media_hangup(struct sipe_account_data *sip);

gchar *sipe_media_get_callid(sipe_media_call *call);


/* Backend functions **********************************************************/

sipe_codec * sipe_backend_codec_new(int id, const char *name,
									SipeMediaType type, guint clock_rate);

void sipe_backend_codec_free(sipe_codec *codec);

int sipe_backend_codec_get_id(sipe_codec *codec);

gchar * sipe_backend_codec_get_name(sipe_codec *codec);

guint sipe_backend_codec_get_clock_rate(sipe_codec *codec);

GList *sipe_backend_codec_get_optional_parameters(sipe_codec *codec);

gboolean sipe_backend_set_remote_codecs(sipe_media_call* call, gchar* participant);

GList* sipe_backend_get_local_codecs(sipe_media_call* call);


sipe_candidate * sipe_backend_candidate_new(const gchar *foundation,
											SipeComponentType component,
											SipeCandidateType type,
											SipeNetworkProtocol proto,
											const gchar *ip, guint port);

void sipe_backend_candidate_free(sipe_candidate *candidate);

void sipe_backend_candidate_set_username_and_pwd(sipe_candidate *candidate,
												 const gchar *username,
												 const gchar *password);
