#include "USBCTRL.hpp"

#include <cstdio>
#include <cstring>
#include "lpc17xx_usb.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

// TSetupPacket USBCTRL::Setup;
// usbdesc_base ** USBCTRL::descriptors;
// usbdesc_device *USBCTRL::dev = (usbdesc_device *) 0;
// usbdesc_configuration *USBCTRL::conf = (usbdesc_configuration *) 0;

// uint8_t USBCTRL::confBuffer[64];
// uint16_t USBCTRL::confSize;
// uint16_t USBCTRL::confRemain;
// uint8_t USBCTRL::confIndex;
// uint8_t USBCTRL::confSubIndex;

// uint8_t bConfiguration;
// uint8_t USBCTRL::bAlternate;

// uint8_t USBCTRL::apbDataStore[4][8] = {
// 	{ 0, 0, 0, 0, 0, 0, 0, 0, },
// 	{ 0, 0, 0, 0, 0, 0, 0, 0, },
// 	{ 0, 0, 0, 0, 0, 0, 0, 0, },
// 	{ 0, 0, 0, 0, 0, 0, 0, 0, },
// };

// uint8_t *USBCTRL::pbData;
// uint16_t USBCTRL::iResidue;
// int USBCTRL::iLen;

USBCTRL::USBCTRL() {
	dev = (usbdesc_device *) 0;
	conf = (usbdesc_configuration *) 0;

	apbDataStore = {
		{ 0, 0, 0, 0, 0, 0, 0, 0, },
		{ 0, 0, 0, 0, 0, 0, 0, 0, },
		{ 0, 0, 0, 0, 0, 0, 0, 0, },
		{ 0, 0, 0, 0, 0, 0, 0, 0, },
	};
}

void USBCTRL::init(usbdesc_base ** d) {
	descriptors = d;
	int confMaxSize = 0;
	conf = (usbdesc_configuration *) 0;

	for (int i = 0; descriptors[i] != (usbdesc_base *) 0; i++) {
		iprintf("descriptor %d\n", i);
		if (d[i]->bDescType == DT_DEVICE)
			dev = (usbdesc_device *) d[i];
		if (d[i]->bDescType == DT_CONFIGURATION) {
			usbdesc_configuration *c = (usbdesc_configuration *) d[i];
			if (c->wTotalLength > confMaxSize)
				confMaxSize = c->wTotalLength;
			if (conf == (usbdesc_configuration *) 0)
				conf = (usbdesc_configuration *) d[i];
		}
	}

	hw.init();

// 	confBuffer = malloc(confMaxSize);

// 	iprintf("registerHandler: Frame\n");
// 	hw.HwRegisterFrameHandler(&FrameHandler);
// 	iprintf("registerHandler: DevInt\n");
// 	hw.HwRegisterDevIntHandler(&DevIntHandler);

// 	iprintf("registerHandler: EP:OUT0\n");
// 	hw.HwRegisterEPIntHandler(0x00, &EPIntHandler);
// 	iprintf("registerHandler: EP:IN0\n");
// 	hw.HwRegisterEPIntHandler(0x80, &EPIntHandler);

// 	iprintf("registerHandler: EP:OUT0Config: 64b\n");
	hw.HwEPConfig(0x00, 64);
// 	iprintf("registerHandler: EP:IN0Config: 64b\n");
	hw.HwEPConfig(0x80, 64);
// 	iprintf("ctrl:init:end\n");
}

void USBCTRL::connect() {
	hw.connect();
}

void USBCTRL::FrameHandler(uint16_t wFrame) {
// 	iprintf("F%4X\t", wFrame);
}

void USBCTRL::DevIntHandler(uint8_t bDevStatus) {
// 	iprintf("D%02X\t", bDevStatus);
}

int USBCTRL::GatherConfigurationDescriptor(int iChunk) {
	pbData = confBuffer;
	uint8_t iBuf = 0;
	uint8_t irmn;
	if (iChunk > confRemain)
		iChunk = confRemain;
	irmn = iChunk;
	while (irmn > 0) {
		uint8_t clen = descriptors[confIndex]->bLength - confSubIndex;
		if (clen == 0) {
			confIndex++;
			confSubIndex = 0;
			// 				iprintf("Next Descriptor %d at %p is %d long!\n", confIndex, &descriptors[confIndex], descriptors[confIndex]->bLength);
			if (descriptors[confIndex] == (usbdesc_base *) 0) {
				confRemain = 0;
				break;
			}
			clen = descriptors[confIndex]->bLength;
		}
		if (clen > irmn)
			clen = irmn;
		memcpy(&confBuffer[iBuf], &((uint8_t *) descriptors[confIndex])[confSubIndex], clen);
		// 			iprintf("Copied %d to %p/%p(%d), %d remains (%d this buffer)\n", clen, &confBuffer[iBuf], pbData, &confBuffer[iBuf] - pbData, confRemain - clen, irmn - clen);
		iBuf += clen;
		confSubIndex += clen;
		irmn -= clen;
		confRemain -= clen;
	}
	iChunk = iBuf;
	return iChunk;
}

void USBCTRL::DataIn() {
	int iChunk = iResidue;
	if (iChunk > 64)
		iChunk = 64;
	if (confRemain > 0)
		iChunk = GatherConfigurationDescriptor(iChunk);
// 	iprintf("W%d/%d:", iChunk, iResidue);
//	for (int i = 0; i < iChunk; i++)
// 		iprintf("0x%02X,", pbData[i]);
// 	iprintf("\n");
	hw.HwEPWrite(0x80, pbData, iChunk);
	pbData += iChunk;
	iResidue -= iChunk;
}

void USBCTRL::EPIntHandler(uint8_t bEP, uint8_t bEPStatus) {
	iprintf("E0x%02X:0x%02X\t", bEP, bEPStatus);
	if (bEP == 0x00) {
		if (bEPStatus & EP_STATUS_SETUP) {
			// defaults for data pointer and residue
			uint8_t iType = REQTYPE_GET_TYPE(Setup.bmRequestType);
			pbData = &apbDataStore[iType][0];
			iResidue = Setup.wLength;
			iLen = Setup.wLength;

			hw.HwEPRead(bEP, (uint8_t *) &Setup, sizeof(Setup));

			if ((Setup.wLength == 0) ||
				(REQTYPE_GET_DIR(Setup.bmRequestType) == REQTYPE_DIR_TO_HOST)) {
				switch(REQTYPE_GET_TYPE(Setup.bmRequestType)) {
					case REQTYPE_TYPE_STANDARD: {
						iprintf("STDREQ:%02X,%02X,%04X,%04X,%04x\n", Setup.bmRequestType, Setup.bRequest, Setup.wValue, Setup.wIndex, Setup.wLength);
						uint8_t r = 0;
						switch (REQTYPE_GET_RECIP(Setup.bmRequestType)) {
							case REQTYPE_RECIP_DEVICE: {
								r = HandleStdDeviceReq(&Setup, &iLen, &pbData);
								iprintf("TS:%d\n", iLen);
								break;
							}
							case REQTYPE_RECIP_INTERFACE: {
								r = HandleStdInterfaceReq(&Setup, &iLen, &pbData);
								break;
							}
							case REQTYPE_RECIP_ENDPOINT: {
								r = HandleStdEndPointReq(&Setup, &iLen, &pbData);
								break;
							}
							case REQTYPE_RECIP_OTHER: {
								break;
							}
						}
						if (!r) {
							hw.HwEPStall(0x80, TRUE);
							return;
						}
						iResidue = iLen;
						if (iResidue > Setup.wLength)
							iResidue = Setup.wLength;

						DataIn();

						break;
					}
				}
			}
		}
		else {
			int iChunk;
			if (iResidue > 0) {
				// store data
				iChunk = hw.HwEPRead(0x00, pbData, iResidue);
				iprintf("G:%db\n", iChunk);
				if (iChunk < 0) {
					hw.HwEPStall(0x80, TRUE);
					return;
				}
				pbData += iChunk;
				iResidue -= iChunk;
				if (iResidue == 0) {
					// received all, send data to handler
					uint8_t iType = REQTYPE_GET_TYPE(Setup.bmRequestType);
					pbData = apbDataStore[iType];
					iprintf("HANDLE 0x%02X with %db\n", iType, iLen);
// 					if (!_HandleRequest(&Setup, &iLen, &pbData)) {
// 						DBG("_HandleRequest2 failed\n");
// 						StallControlPipe(bEPStat);
// 						return;
// 					}
					// send status to host
					DataIn();
				}
			}
			else {
				// absorb zero-length status message
				iChunk = hw.HwEPRead(0x00, NULL, 0);
				iprintf("g%db\n", iChunk);
// 				DBG(iChunk > 0 ? "?" : "");
			}
		}
	}
	else if (bEP == 0x80) {
		DataIn();
	}
}



/**
 * Parses the list of installed USB descriptors and attempts to find
 * the specified USB descriptor.
 *
 * @param [in]		wTypeIndex	Type and index of the descriptor
 * @param [in]		wLangID		Language ID of the descriptor (currently unused)
 * @param [out]	*piLen		Descriptor length
 * @param [out]	*ppbData	Descriptor data
 *
 * @return TRUE if the descriptor was found, FALSE otherwise
 */
uint8_t USBCTRL::GetDescriptor(uint16_t wTypeIndex, uint16_t wLangID, int *piLen, uint8_t **ppbData) {
	uint8_t	bType, bIndex;
	int iCurIndex;

// 	ASSERT(pabDescrip != NULL);

	bType = GET_DESC_TYPE(wTypeIndex);
	bIndex = GET_DESC_INDEX(wTypeIndex);

	iCurIndex = 0;
	for (int i = 0; descriptors[i] != (usbdesc_base *) 0; i++) {
		if (descriptors[i]->bDescType == bType) {
			if (iCurIndex == bIndex) {
				*ppbData = (uint8_t *) descriptors[i];
				if (bType == DT_CONFIGURATION) {
					confSize = ((usbdesc_configuration *) descriptors[i])->wTotalLength;
					confRemain = confSize;
					confIndex = i;
					confSubIndex = 0;
					*piLen = ((usbdesc_configuration *) descriptors[i])->wTotalLength;
					*ppbData = confBuffer;
					iprintf("Get Configuration: %d bytes!\n", *piLen);
				}
				else {
					*piLen = descriptors[i]->bLength;
					confRemain = 0;
					iprintf("Get 0x%02X: %d bytes\n", bIndex, *piLen);
				}
				return TRUE;
			}
			iCurIndex++;
		}
	}
	iprintf("Descriptor matching 0x%02X not found\n", wTypeIndex);
// 	pab = (uint8_t *)pabDescrip;
// 	iCurIndex = 0;
//
// 	while (pab[DESC_bLength] != 0) {
// 		if (pab[DESC_bDescriptorType] == bType) {
// 			if (iCurIndex == bIndex) {
// 				// set data pointer
// 				*ppbData = pab;
// 				// get length from structure
// 				if (bType == DESC_CONFIGURATION) {
// 					// configuration descriptor is an exception, length is at offset 2 and 3
// 					*piLen =	(pab[CONF_DESC_wTotalLength]) |
// 					(pab[CONF_DESC_wTotalLength + 1] << 8);
// 				}
// 				else {
// 					// normally length is at offset 0
// 					*piLen = pab[DESC_bLength];
// 				}
// 				return TRUE;
// 			}
// 			iCurIndex++;
// 		}
// 		// skip to next descriptor
// 		pab += pab[DESC_bLength];
// 	}
	// nothing found
// 	DBG("Desc %x/%x not found!\n", GET_DESC_TYPE(wTypeIndex), GET_DESC_INDEX(wTypeIndex));
	*piLen = 0;
	return TRUE;
}


/**
 * Configures the device according to the specified configuration index and
 * alternate setting by parsing the installed USB descriptor list.
 * A configuration index of 0 unconfigures the device.
 *
 * @param [in]		bConfigIndex	Configuration index
 * @param [in]		bAltSetting		Alternate setting number
 *
 * @todo function always returns TRUE, add stricter checking?
 *
 * @return TRUE if successfully configured, FALSE otherwise
 */
uint8_t USBCTRL::SetConfiguration(uint8_t bConfigIndex, uint8_t bAltSetting) {
	uint8_t	bCurConfig, bCurAltSetting;

// 	ASSERT(pabDescrip != NULL);

	if (bConfigIndex == 0) {
		// unconfigure device
		hw.HwConfigDevice(FALSE);
	}
	else {
		bCurConfig = 0xFF;
		bCurAltSetting = 0xFF;

		for (int i = 0; descriptors[i] != (usbdesc_base *) 0; i++) {
			switch(descriptors[i]->bDescType) {
				case DT_CONFIGURATION: {
					usbdesc_configuration *c = (usbdesc_configuration *) descriptors[i];
					bCurConfig = c->bConfigurationValue;
					if (bCurConfig == bConfigIndex)
						conf = c;
					break;
				}
				case DT_INTERFACE: {
					usbdesc_interface *inf = (usbdesc_interface *) descriptors[i];
					bCurAltSetting = inf->bAlternateSetting;
					break;
				}
				case DT_ENDPOINT: {
					usbdesc_endpoint *e = (usbdesc_endpoint *) descriptors[i];
					if ((bCurConfig == bConfigIndex) && (bCurAltSetting == bAltSetting)) {
						hw.HwEPConfig(e->bEndpointAddress, e->wMaxPacketSize);
						bAlternate = bAltSetting;
					}
					break;
				}
			}
		}
		// configure endpoints for this configuration/altsetting
// 		pab = (uint8_t *)pabDescrip;
// 		bCurConfig = 0xFF;
// 		bCurAltSetting = 0xFF;
//
// 		while (pab[DESC_bLength] != 0) {
//
// 			switch (pab[DESC_bDescriptorType]) {
//
// 				case DESC_CONFIGURATION:
// 					// remember current configuration index
// 					bCurConfig = pab[CONF_DESC_bConfigurationValue];
// 					break;
//
// 				case DESC_INTERFACE:
// 					// remember current alternate setting
// 					bCurAltSetting = pab[INTF_DESC_bAlternateSetting];
// 					break;
//
// 				case DESC_ENDPOINT:
// 					bEP = pab[ENDP_DESC_bEndpointAddress];
// // 					DBG("Check EP %x %d/%d %d/%d:", bEP, bCurConfig, bConfigIndex, bCurAltSetting, bAltSetting);
// 					if ((bCurConfig == bConfigIndex) &&
// 						(bCurAltSetting == bAltSetting)) {
// // 						DBG(" Found!\n");
// 					// endpoint found for desired config and alternate setting
// 					wMaxPktSize = 	(pab[ENDP_DESC_wMaxPacketSize]) |
// 					(pab[ENDP_DESC_wMaxPacketSize + 1] << 8);
// 					// configure endpoint
// 					hw.HwEPConfig(bEP, wMaxPktSize);
// 						}
// // 						else {
// // 							DBG(" No Match\n");
// // 						}
// 						break;
//
// 				default:
// 					break;
// 			}
// 			// skip to next descriptor
// 			pab += pab[DESC_bLength];
// 		}

		// configure device
		hw.HwConfigDevice(TRUE);
	}

	return TRUE;
}


/**
 * Local function to handle a standard device request
 *
 * @param [in]		pSetup		The setup packet
 * @param [in,out]	*piLen		Pointer to data length
 * @param [in,out]	ppbData		Data buffer.
 *
 * @return TRUE if the request was handled successfully
 */
uint8_t USBCTRL::HandleStdDeviceReq(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData) {
	uint8_t	*pbData = *ppbData;

	iResidue = 0;

	switch (pSetup->bRequest) {
		case REQ_GET_STATUS:
			iprintf("GET_STATUS\n");
			// bit 0: self-powered
			// bit 1: remote wakeup = not supported
			pbData[0] = 0;
			pbData[1] = 0;
			*piLen = 2;
			break;

		case REQ_SET_ADDRESS:
			iprintf("USBADDR %d!\n", pSetup->wValue);
			hw.HwSetAddress(pSetup->wValue);
			*piLen = 0;
			break;

		case REQ_GET_DESCRIPTOR:
			iprintf("GET DESCRIPTOR 0x%02X/0x%02X\n", GET_DESC_TYPE(pSetup->wValue), GET_DESC_INDEX(pSetup->wValue));
// 			DBG("D%x", pSetup->wValue);
			return GetDescriptor(pSetup->wValue, pSetup->wIndex, piLen, ppbData);

		case REQ_GET_CONFIGURATION:
			iprintf("GET CONFIGURATION\n");
			// indicate if we are configured
			pbData[0] = conf->bConfigurationValue;
			*piLen = 1;
			break;

		case REQ_SET_CONFIGURATION:
			iprintf("SET CONFIGURATION\n");
			*piLen = 0;
			if (!SetConfiguration(pSetup->wValue & 0xFF, bAlternate)) {
// 				DBG("USBSetConfiguration failed!\n");
				return FALSE;
			}
			// configuration successful, update current configuration
// 			bConfiguration = pSetup->wValue & 0xFF;
			break;

		case REQ_CLEAR_FEATURE:
		case REQ_SET_FEATURE:
			*piLen = 0;
			iprintf("CLEAR/SET FEATURE\n");
			if (pSetup->wValue == FEA_REMOTE_WAKEUP) {
				// put DEVICE_REMOTE_WAKEUP code here
			}
			if (pSetup->wValue == FEA_TEST_MODE) {
				// put TEST_MODE code here
			}
			return FALSE;

		case REQ_SET_DESCRIPTOR:
			*piLen = 0;
			iprintf("SET DESCRIPTOR\n");
// 			DBG("Device req %d not implemented\n", pSetup->bRequest);
			return FALSE;

		default:
			*piLen = 0;
			iprintf("UNKNOWN: 0x%02X\n", pSetup->bRequest);
// 			DBG("Illegal device req %d\n", pSetup->bRequest);
			return FALSE;
	}

	return TRUE;
}


/**
 * Local function to handle a standard interface request
 *
 * @param [in]		pSetup		The setup packet
 * @param [in,out]	*piLen		Pointer to data length
 * @param [in]		ppbData		Data buffer.
 *
 * @return TRUE if the request was handled successfully
 */
uint8_t USBCTRL::HandleStdInterfaceReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData) {
	uint8_t	*pbData = *ppbData;

	switch (pSetup->bRequest) {
		case REQ_GET_STATUS:
			// no bits specified
			pbData[0] = 0;
			pbData[1] = 0;
			*piLen = 2;
			break;

		case REQ_CLEAR_FEATURE:
		case REQ_SET_FEATURE:
			// not defined for interface
			return FALSE;

		case REQ_GET_INTERFACE:	// TODO use bNumInterfaces
        // there is only one interface, return n-1 (= 0)
        pbData[0] = bAlternate;
		*piLen = 1;
		break;

		case REQ_SET_INTERFACE:	// TODO use bNumInterfaces
		// there is only one interface (= 0)
		if (pSetup->wValue > conf->bNumInterfaces) {
			return FALSE;
		}
// 		DBG("Set alternate %d", pSetup->wValue);
		if (SetConfiguration(conf->bConfigurationValue, pSetup->wValue)) {
// 			DBG(" OK\n");
// 			bAlternate = pSetup->wValue;
		}
		else {
// 			DBG(" failed!\n");
			return FALSE;
		}
		*piLen = 0;
		break;

		default:
// 			DBG("Illegal interface req %d\n", pSetup->bRequest);
			return FALSE;
	}

	return TRUE;
}


/**
 * Local function to handle a standard endpoint request
 *
 * @param [in]		pSetup		The setup packet
 * @param [in,out]	*piLen		Pointer to data length
 * @param [in]		ppbData		Data buffer.
 *
 * @return TRUE if the request was handled successfully
 */
uint8_t USBCTRL::HandleStdEndPointReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData) {
	uint8_t	*pbData = *ppbData;

	switch (pSetup->bRequest) {
		case REQ_GET_STATUS:
			// bit 0 = endpointed halted or not
			pbData[0] = (hw.HwEPGetStatus(pSetup->wIndex) & EP_STATUS_STALLED) ? 1 : 0;
			pbData[1] = 0;
			*piLen = 2;
			break;

		case REQ_CLEAR_FEATURE:
			if (pSetup->wValue == FEA_ENDPOINT_HALT) {
				// clear HALT by unstalling
				hw.HwEPStall(pSetup->wIndex, FALSE);
				break;
			}
			// only ENDPOINT_HALT defined for endpoints
			return FALSE;

		case REQ_SET_FEATURE:
			if (pSetup->wValue == FEA_ENDPOINT_HALT) {
				// set HALT by stalling
				hw.HwEPStall(pSetup->wIndex, TRUE);
				break;
			}
			// only ENDPOINT_HALT defined for endpoints
			return FALSE;

		case REQ_SYNCH_FRAME:
// 			DBG("EP req %d not implemented\n", pSetup->bRequest);
			return FALSE;

		default:
// 			DBG("Illegal EP req %d\n", pSetup->bRequest);
			return FALSE;
	}

	return TRUE;
}


/**
 * Default handler for standard ('chapter 9') requests
 *
 * If a custom request handler was installed, this handler is called first.
 *
 * @param [in]		pSetup		The setup packet
 * @param [in,out]	*piLen		Pointer to data length
 * @param [in]		ppbData		Data buffer.
 *
 * @return TRUE if the request was handled successfully
 */
uint8_t USBCTRL::HandleStandardRequest(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData) {
	// try the custom request handler first
	if ((pfnHandleCustomReq != NULL) && pfnHandleCustomReq(pSetup, piLen, ppbData)) {
		return TRUE;
	}

	switch (REQTYPE_GET_RECIP(pSetup->bmRequestType)) {
		case REQTYPE_RECIP_DEVICE:		return HandleStdDeviceReq(pSetup, piLen, ppbData);
		case REQTYPE_RECIP_INTERFACE:	return HandleStdInterfaceReq(pSetup, piLen, ppbData);
		case REQTYPE_RECIP_ENDPOINT: 	return HandleStdEndPointReq(pSetup, piLen, ppbData);
		default: 						return FALSE;
	}
}
