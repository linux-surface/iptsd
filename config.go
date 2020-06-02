package main

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"github.com/pkg/errors"
	"gopkg.in/ini.v1"
)

type IptsConfig struct {
	InvertX bool
	InvertY bool

	Width  int
	Height int
}

func (cfg *IptsConfig) LoadFromDir(info *IptsDeviceInfo, dir string) error {
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		return nil
	}

	files, err := ioutil.ReadDir(dir)
	if err != nil {
		return errors.WithStack(err)
	}

	for _, f := range files {
		if f.IsDir() {
			continue
		}

		if !strings.HasSuffix(f.Name(), ".conf") {
			continue
		}

		file, err := ini.Load(filepath.Join(dir, f.Name()))
		if err != nil {
			return errors.WithStack(err)
		}

		section := file.Section("Device")
		if section == nil {
			continue
		}

		venKey := section.Key("Vendor")
		if venKey == nil {
			continue
		}

		proKey := section.Key("Product")
		if proKey == nil {
			continue
		}

		ven, err := venKey.Uint()
		if err != nil {
			return errors.WithStack(err)
		}

		pro, err := proKey.Uint()
		if err != nil {
			return errors.WithStack(err)
		}

		if ven != uint(info.Vendor) || pro != uint(info.Product) {
			continue
		}

		section = file.Section("Config")
		if section == nil {
			continue
		}

		err = section.MapTo(cfg)
		if err != nil {
			return errors.WithStack(err)
		}

		return nil
	}

	return nil
}

func (cfg *IptsConfig) Load(info *IptsDeviceInfo) error {
	err := cfg.LoadFromDir(info, "/usr/share/ipts")
	if err != nil {
		return err
	}

	err = cfg.LoadFromDir(info, "/usr/local/share/ipts")
	if err != nil {
		return err
	}

	err = cfg.LoadFromDir(info, "./config")
	if err != nil {
		return err
	}

	if _, err := os.Stat("/etc/ipts.conf"); os.IsNotExist(err) {
		return nil
	}

	file, err := ini.Load("/etc/ipts.conf")
	if err != nil {
		return errors.WithStack(err)
	}

	section := file.Section("Config")
	if section == nil {
		return nil
	}

	err = section.MapTo(cfg)
	if err != nil {
		return errors.WithStack(err)
	}

	return nil
}
