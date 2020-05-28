package main

func IptsSingletouchHandleInput(ipts *IptsContext) error {
	touch := ipts.Devices.Touch

	data, err := ipts.Protocol.ReadSingletouchData()
	if err != nil {
		return err
	}

	touch.Emit(EV_ABS, ABS_MT_SLOT, 0)

	if data.Touch > 0 {
		touch.Emit(EV_ABS, ABS_MT_TRACKING_ID, 0)
		touch.Emit(EV_ABS, ABS_MT_POSITION_X, int32(data.X))
		touch.Emit(EV_ABS, ABS_MT_POSITION_Y, int32(data.Y))
	} else {
		touch.Emit(EV_ABS, ABS_MT_TRACKING_ID, -1)
	}

	err = touch.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}
