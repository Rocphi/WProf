// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/resource_creation_proxy.h"

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_size.h"
#include "ppapi/c/trusted/ppb_image_data_trusted.h"
#include "ppapi/proxy/plugin_dispatcher.h"
#include "ppapi/proxy/plugin_resource_tracker.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/ppb_audio_input_proxy.h"
#include "ppapi/proxy/ppb_audio_proxy.h"
#include "ppapi/proxy/ppb_buffer_proxy.h"
#include "ppapi/proxy/ppb_broker_proxy.h"
#include "ppapi/proxy/ppb_file_chooser_proxy.h"
#include "ppapi/proxy/ppb_file_io_proxy.h"
#include "ppapi/proxy/ppb_file_ref_proxy.h"
#include "ppapi/proxy/ppb_file_system_proxy.h"
#include "ppapi/proxy/ppb_flash_device_id_proxy.h"
#include "ppapi/proxy/ppb_flash_menu_proxy.h"
#include "ppapi/proxy/ppb_flash_message_loop_proxy.h"
#include "ppapi/proxy/ppb_graphics_2d_proxy.h"
#include "ppapi/proxy/ppb_graphics_3d_proxy.h"
#include "ppapi/proxy/ppb_host_resolver_private_proxy.h"
#include "ppapi/proxy/ppb_image_data_proxy.h"
#include "ppapi/proxy/ppb_network_monitor_private_proxy.h"
#include "ppapi/proxy/ppb_talk_private_proxy.h"
#include "ppapi/proxy/ppb_tcp_server_socket_private_proxy.h"
#include "ppapi/proxy/ppb_tcp_socket_private_proxy.h"
#include "ppapi/proxy/ppb_udp_socket_private_proxy.h"
#include "ppapi/proxy/ppb_url_loader_proxy.h"
#include "ppapi/proxy/ppb_video_capture_proxy.h"
#include "ppapi/proxy/ppb_video_decoder_proxy.h"
#include "ppapi/proxy/ppb_x509_certificate_private_proxy.h"
#include "ppapi/shared_impl/api_id.h"
#include "ppapi/shared_impl/host_resource.h"
#include "ppapi/shared_impl/ppb_audio_config_shared.h"
#include "ppapi/shared_impl/ppb_input_event_shared.h"
#include "ppapi/shared_impl/ppb_resource_array_shared.h"
#include "ppapi/shared_impl/ppb_url_request_info_shared.h"
#include "ppapi/shared_impl/private/ppb_browser_font_trusted_shared.h"
#include "ppapi/shared_impl/var.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_image_data_api.h"

using ppapi::thunk::ResourceCreationAPI;

namespace ppapi {
namespace proxy {

ResourceCreationProxy::ResourceCreationProxy(Dispatcher* dispatcher)
    : InterfaceProxy(dispatcher) {
}

ResourceCreationProxy::~ResourceCreationProxy() {
}

// static
InterfaceProxy* ResourceCreationProxy::Create(Dispatcher* dispatcher) {
  return new ResourceCreationProxy(dispatcher);
}

PP_Resource ResourceCreationProxy::CreateFileIO(PP_Instance instance) {
  return PPB_FileIO_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateFileRef(PP_Resource file_system,
                                                 const char* path) {
  return PPB_FileRef_Proxy::CreateProxyResource(file_system, path);
}

PP_Resource ResourceCreationProxy::CreateFileSystem(
    PP_Instance instance,
    PP_FileSystemType type) {
  return PPB_FileSystem_Proxy::CreateProxyResource(instance, type);
}

PP_Resource ResourceCreationProxy::CreateIMEInputEvent(
    PP_Instance instance,
    PP_InputEvent_Type type,
    PP_TimeTicks time_stamp,
    struct PP_Var text,
    uint32_t segment_number,
    const uint32_t* segment_offsets,
    int32_t target_segment,
    uint32_t selection_start,
    uint32_t selection_end) {
  return PPB_InputEvent_Shared::CreateIMEInputEvent(
      OBJECT_IS_PROXY, instance, type, time_stamp, text, segment_number,
      segment_offsets, target_segment, selection_start, selection_end);
}

PP_Resource ResourceCreationProxy::CreateKeyboardInputEvent(
    PP_Instance instance,
    PP_InputEvent_Type type,
    PP_TimeTicks time_stamp,
    uint32_t modifiers,
    uint32_t key_code,
    struct PP_Var character_text) {
  return PPB_InputEvent_Shared::CreateKeyboardInputEvent(
      OBJECT_IS_PROXY, instance, type, time_stamp, modifiers, key_code,
      character_text);
}

PP_Resource ResourceCreationProxy::CreateMouseInputEvent(
    PP_Instance instance,
    PP_InputEvent_Type type,
    PP_TimeTicks time_stamp,
    uint32_t modifiers,
    PP_InputEvent_MouseButton mouse_button,
    const PP_Point* mouse_position,
    int32_t click_count,
    const PP_Point* mouse_movement) {
  return PPB_InputEvent_Shared::CreateMouseInputEvent(
      OBJECT_IS_PROXY, instance, type, time_stamp, modifiers,
      mouse_button, mouse_position, click_count, mouse_movement);
}

PP_Resource ResourceCreationProxy::CreateTouchInputEvent(
    PP_Instance instance,
    PP_InputEvent_Type type,
    PP_TimeTicks time_stamp,
    uint32_t modifiers) {
  return PPB_InputEvent_Shared::CreateTouchInputEvent(
      OBJECT_IS_PROXY, instance, type, time_stamp, modifiers);
}

PP_Resource ResourceCreationProxy::CreateResourceArray(
    PP_Instance instance,
    const PP_Resource elements[],
    uint32_t size) {
  PPB_ResourceArray_Shared* object = new PPB_ResourceArray_Shared(
      OBJECT_IS_PROXY, instance, elements, size);
  return object->GetReference();
}

PP_Resource ResourceCreationProxy::CreateURLLoader(PP_Instance instance) {
  return PPB_URLLoader_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateURLRequestInfo(
    PP_Instance instance,
    const PPB_URLRequestInfo_Data& data) {
  return (new PPB_URLRequestInfo_Shared(OBJECT_IS_PROXY,
                                        instance, data))->GetReference();
}

PP_Resource ResourceCreationProxy::CreateWheelInputEvent(
    PP_Instance instance,
    PP_TimeTicks time_stamp,
    uint32_t modifiers,
    const PP_FloatPoint* wheel_delta,
    const PP_FloatPoint* wheel_ticks,
    PP_Bool scroll_by_page) {
  return PPB_InputEvent_Shared::CreateWheelInputEvent(
      OBJECT_IS_PROXY, instance, time_stamp, modifiers,
      wheel_delta, wheel_ticks, scroll_by_page);
}

PP_Resource ResourceCreationProxy::CreateAudio(
    PP_Instance instance,
    PP_Resource config_id,
    PPB_Audio_Callback audio_callback,
    void* user_data) {
  return PPB_Audio_Proxy::CreateProxyResource(instance, config_id,
                                              audio_callback, user_data);
}

PP_Resource ResourceCreationProxy::CreateAudioConfig(
    PP_Instance instance,
    PP_AudioSampleRate sample_rate,
    uint32_t sample_frame_count) {
  return PPB_AudioConfig_Shared::Create(
      OBJECT_IS_PROXY, instance, sample_rate, sample_frame_count);
}

PP_Resource ResourceCreationProxy::CreateImageData(PP_Instance instance,
                                                   PP_ImageDataFormat format,
                                                   const PP_Size& size,
                                                   PP_Bool init_to_zero) {
  return PPB_ImageData_Proxy::CreateProxyResource(instance, format, size,
                                                  init_to_zero);
}

PP_Resource ResourceCreationProxy::CreateGraphics2D(PP_Instance instance,
                                                    const PP_Size& size,
                                                    PP_Bool is_always_opaque) {
  return PPB_Graphics2D_Proxy::CreateProxyResource(instance, size,
                                                   is_always_opaque);
}

#if !defined(OS_NACL)
PP_Resource ResourceCreationProxy::CreateAudioTrusted(PP_Instance instance) {
  // Proxied plugins can't create trusted audio devices.
  return 0;
}

PP_Resource ResourceCreationProxy::CreateAudioInput0_1(
    PP_Instance instance,
    PP_Resource config_id,
    PPB_AudioInput_Callback audio_input_callback,
    void* user_data) {
  return PPB_AudioInput_Proxy::CreateProxyResource0_1(instance, config_id,
                                                      audio_input_callback,
                                                      user_data);
}

PP_Resource ResourceCreationProxy::CreateAudioInput(
    PP_Instance instance) {
  return PPB_AudioInput_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateBroker(PP_Instance instance) {
  return PPB_Broker_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateBrowserFont(
    PP_Instance instance,
    const PP_BrowserFont_Trusted_Description* description) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return 0;
  return PPB_BrowserFont_Trusted_Shared::Create(
      OBJECT_IS_PROXY, instance, *description, dispatcher->preferences());
}

PP_Resource ResourceCreationProxy::CreateBuffer(PP_Instance instance,
                                                uint32_t size) {
  return PPB_Buffer_Proxy::CreateProxyResource(instance, size);
}

PP_Resource ResourceCreationProxy::CreateDirectoryReader(
    PP_Resource directory_ref) {
  NOTIMPLEMENTED();  // Not proxied yet.
  return 0;
}

PP_Resource ResourceCreationProxy::CreateFileChooser(
    PP_Instance instance,
    PP_FileChooserMode_Dev mode,
    const char* accept_types) {
  return PPB_FileChooser_Proxy::CreateProxyResource(instance, mode,
                                                    accept_types);
}

PP_Resource ResourceCreationProxy::CreateFlashDeviceID(PP_Instance instance) {
  return PPB_Flash_DeviceID_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateFlashMenu(
    PP_Instance instance,
    const PP_Flash_Menu* menu_data) {
  return PPB_Flash_Menu_Proxy::CreateProxyResource(instance, menu_data);
}

PP_Resource ResourceCreationProxy::CreateFlashMessageLoop(
    PP_Instance instance) {
  return PPB_Flash_MessageLoop_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateHostResolverPrivate(
    PP_Instance instance) {
  return PPB_HostResolver_Private_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateNetworkMonitor(
      PP_Instance instance,
      PPB_NetworkMonitor_Callback callback,
      void* user_data) {
  return PPB_NetworkMonitor_Private_Proxy::CreateProxyResource(
      instance, callback, user_data);
}

PP_Resource ResourceCreationProxy::CreateGraphics3D(
    PP_Instance instance,
    PP_Resource share_context,
    const int32_t* attrib_list) {
  return PPB_Graphics3D_Proxy::CreateProxyResource(
      instance, share_context, attrib_list);
}

PP_Resource ResourceCreationProxy::CreateGraphics3DRaw(
    PP_Instance instance,
    PP_Resource share_context,
    const int32_t* attrib_list) {
  // Not proxied. The raw creation function is used only in the implementation
  // of the proxy on the host side.
  return 0;
}

PP_Resource ResourceCreationProxy::CreateScrollbar(PP_Instance instance,
                                                   PP_Bool vertical) {
  NOTIMPLEMENTED();  // Not proxied yet.
  return 0;
}

PP_Resource ResourceCreationProxy::CreateTalk(PP_Instance instance) {
  return PPB_Talk_Private_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateTCPServerSocketPrivate(
    PP_Instance instance) {
  return PPB_TCPServerSocket_Private_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateTCPSocketPrivate(
    PP_Instance instance) {
  return PPB_TCPSocket_Private_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateUDPSocketPrivate(
    PP_Instance instance) {
  return PPB_UDPSocket_Private_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateVideoCapture(PP_Instance instance) {
  return PPB_VideoCapture_Proxy::CreateProxyResource(instance);
}

PP_Resource ResourceCreationProxy::CreateVideoDecoder(
    PP_Instance instance,
    PP_Resource context3d_id,
    PP_VideoDecoder_Profile profile) {
  return PPB_VideoDecoder_Proxy::CreateProxyResource(
      instance, context3d_id, profile);
}

PP_Resource ResourceCreationProxy::CreateVideoLayer(
    PP_Instance instance,
    PP_VideoLayerMode_Dev mode) {
  NOTIMPLEMENTED();
  return 0;
}

PP_Resource ResourceCreationProxy::CreateWebSocket(PP_Instance instance) {
  NOTIMPLEMENTED();
  return 0;
}

PP_Resource ResourceCreationProxy::CreateX509CertificatePrivate(
    PP_Instance instance) {
  return PPB_X509Certificate_Private_Proxy::CreateProxyResource(instance);
}
#endif  // !defined(OS_NACL)


bool ResourceCreationProxy::Send(IPC::Message* msg) {
  return dispatcher()->Send(msg);
}

bool ResourceCreationProxy::OnMessageReceived(const IPC::Message& msg) {
  return false;
}

}  // namespace proxy
}  // namespace ppapi
