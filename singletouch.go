package main

func IptsSingletouchHandleInput(ipts *IptsContext) error {
	singletouch := ipts.Devices.Singletouch

	data, err := ipts.Protocol.ReadSingletouchData()
	if err != nil {
		return err
	}

	singletouch.Emit(EV_KEY, BTN_TOUCH, int32(data.Touch))
	singletouch.Emit(EV_ABS, ABS_X, int32(data.X))
	singletouch.Emit(EV_ABS, ABS_Y, int32(data.Y))

	err = singletouch.Emit(EV_SYN, SYN_REPORT, 0)
	if err != nil {
		return err
	}

	return nil
}
