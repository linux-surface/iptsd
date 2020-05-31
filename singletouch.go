package main

func IptsSingletouchHandleInput(ipts *IptsContext) error {
	touch := ipts.Devices.Touch

	data, err := ipts.Protocol.ReadSingletouchData()
	if err != nil {
		return err
	}

	x := float32(data.X) / 32767 * 9600
	y := float32(data.Y) / 32762 * 7200

	touch.Device.Emit(EV_ABS, ABS_MT_SLOT, 0)

	if data.Touch > 0 {
		touch.Device.Emit(EV_ABS, ABS_MT_TRACKING_ID, 0)
		touch.Device.Emit(EV_ABS, ABS_MT_POSITION_X, int32(x))
		touch.Device.Emit(EV_ABS, ABS_MT_POSITION_Y, int32(y))
	} else {
		touch.Device.Emit(EV_ABS, ABS_MT_TRACKING_ID, -1)
	}

	err = touch.Device.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}
