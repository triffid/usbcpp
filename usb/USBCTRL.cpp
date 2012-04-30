#include "USBCTRL.hpp"

#include <cstdio>
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

USBHW * USBCTRL::hw;
USB * USBCTRL::u;
TSetupPacket USBCTRL::Setup;

USBCTRL::USBCTRL() {}

void USBCTRL::init(USB *usb, USBHW *hwp) {
	u = usb;
	hw = hwp;
	hw->HwRegisterFrameHandler(&FrameHandler);
	hw->HwRegisterDevIntHandler(&DevIntHandler);
// 	hw->HwRegisterEpIntHandler(&EPIntHandler);
	hw->HwRegisterEPIntHandler(0x00, &EPIntHandler);
	hw->HwRegisterEPIntHandler(0x80, &EPIntHandler);
}

void USBCTRL::FrameHandler(uint16_t wFrame) {
	printf("F%4X\t", wFrame);
}

void USBCTRL::DevIntHandler(uint8_t bDevStatus) {
	printf("D%02X\t", bDevStatus);
}

void USBCTRL::EPIntHandler(uint8_t bEP, uint8_t bEPStatus) {
// 	printf("E0x%02X:%02X\t", bEP, bEPStatus);
	if (bEP == 0x00) {
		if (bEPStatus & EP_STATUS_SETUP) {
			hw->HwEPRead(bEP, (uint8_t *) &Setup, sizeof(Setup));
		}
	}
	else if (bEP == 0x80) {
	}
	else {
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
	uint8_t	*pab;
	int iCurIndex;

// 	ASSERT(pabDescrip != NULL);

	bType = GET_DESC_TYPE(wTypeIndex);
	bIndex = GET_DESC_INDEX(wTypeIndex);

	pab = (uint8_t *)pabDescrip;
	iCurIndex = 0;

	while (pab[DESC_bLength] != 0) {
		if (pab[DESC_bDescriptorType] == bType) {
			if (iCurIndex == bIndex) {
				// set data pointer
				*ppbData = pab;
				// get length from structure
				if (bType == DESC_CONFIGURATION) {
					// configuration descriptor is an exception, length is at offset 2 and 3
					*piLen =	(pab[CONF_DESC_wTotalLength]) |
					(pab[CONF_DESC_wTotalLength + 1] << 8);
				}
				else {
					// normally length is at offset 0
					*piLen = pab[DESC_bLength];
				}
				return TRUE;
			}
			iCurIndex++;
		}
		// skip to next descriptor
		pab += pab[DESC_bLength];
	}
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
	uint8_t	*pab;
	uint8_t	bCurConfig, bCurAltSetting;
	uint8_t	bEP;
	uint16_t	wMaxPktSize;

// 	ASSERT(pabDescrip != NULL);

	if (bConfigIndex == 0) {
		// unconfigure device
		hw->HwConfigDevice(FALSE);
	}
	else {
		// configure endpoints for this configuration/altsetting
		pab = (uint8_t *)pabDescrip;
		bCurConfig = 0xFF;
		bCurAltSetting = 0xFF;

		while (pab[DESC_bLength] != 0) {

			switch (pab[DESC_bDescriptorType]) {

				case DESC_CONFIGURATION:
					// remember current configuration index
					bCurConfig = pab[CONF_DESC_bConfigurationValue];
					break;

				case DESC_INTERFACE:
					// remember current alternate setting
					bCurAltSetting = pab[INTF_DESC_bAlternateSetting];
					break;

				case DESC_ENDPOINT:
					bEP = pab[ENDP_DESC_bEndpointAddress];
// 					DBG("Check EP %x %d/%d %d/%d:", bEP, bCurConfig, bConfigIndex, bCurAltSetting, bAltSetting);
					if ((bCurConfig == bConfigIndex) &&
						(bCurAltSetting == bAltSetting)) {
// 						DBG(" Found!\n");
					// endpoint found for desired config and alternate setting
					wMaxPktSize = 	(pab[ENDP_DESC_wMaxPacketSize]) |
					(pab[ENDP_DESC_wMaxPacketSize + 1] << 8);
					// configure endpoint
					hw->HwEPConfig(bEP, wMaxPktSize);
						}
// 						else {
// 							DBG(" No Match\n");
// 						}
						break;

				default:
					break;
			}
			// skip to next descriptor
			pab += pab[DESC_bLength];
		}

		// configure device
		hw->HwConfigDevice(TRUE);
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

	switch (pSetup->bRequest) {

		case REQ_GET_STATUS:
			// bit 0: self-powered
			// bit 1: remote wakeup = not supported
			pbData[0] = 0;
			pbData[1] = 0;
			*piLen = 2;
			break;

		case REQ_SET_ADDRESS:
			hw->HwSetAddress(pSetup->wValue);
			break;

		case REQ_GET_DESCRIPTOR:
// 			DBG("D%x", pSetup->wValue);
			return GetDescriptor(pSetup->wValue, pSetup->wIndex, piLen, ppbData);

		case REQ_GET_CONFIGURATION:
			// indicate if we are configured
			pbData[0] = bConfiguration;
			*piLen = 1;
			break;

		case REQ_SET_CONFIGURATION:
			if (!SetConfiguration(pSetup->wValue & 0xFF, bAlternate)) {
// 				DBG("USBSetConfiguration failed!\n");
				return FALSE;
			}
			// configuration successful, update current configuration
			bConfiguration = pSetup->wValue & 0xFF;
			break;

		case REQ_CLEAR_FEATURE:
		case REQ_SET_FEATURE:
			if (pSetup->wValue == FEA_REMOTE_WAKEUP) {
				// put DEVICE_REMOTE_WAKEUP code here
			}
			if (pSetup->wValue == FEA_TEST_MODE) {
				// put TEST_MODE code here
			}
			return FALSE;

		case REQ_SET_DESCRIPTOR:
// 			DBG("Device req %d not implemented\n", pSetup->bRequest);
			return FALSE;

		default:
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
		if (pSetup->wValue > u->getConf()->bNumInterfaces) {
			return FALSE;
		}
// 		DBG("Set alternate %d", pSetup->wValue);
		if (SetConfiguration(bConfiguration, pSetup->wValue)) {
// 			DBG(" OK\n");
			bAlternate = pSetup->wValue;
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
			pbData[0] = (hw->HwEPGetStatus(pSetup->wIndex) & EP_STATUS_STALLED) ? 1 : 0;
			pbData[1] = 0;
			*piLen = 2;
			break;

		case REQ_CLEAR_FEATURE:
			if (pSetup->wValue == FEA_ENDPOINT_HALT) {
				// clear HALT by unstalling
				hw->HwEPStall(pSetup->wIndex, FALSE);
				break;
			}
			// only ENDPOINT_HALT defined for endpoints
			return FALSE;

		case REQ_SET_FEATURE:
			if (pSetup->wValue == FEA_ENDPOINT_HALT) {
				// set HALT by stalling
				hw->HwEPStall(pSetup->wIndex, TRUE);
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
