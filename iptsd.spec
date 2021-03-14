%global debug_package %{nil}

Name: iptsd
Version: 0.4
Release: 1%{?dist}
Summary: Userspace daemon for Intel Precise Touch & Stylus
License: GPLv2+

URL: https://github.com/linux-surface/iptsd
Source: {{{ git_dir_pack dir_name="iptsd" source_name="iptsd.tar.gz"  }}}

BuildRequires: meson
BuildRequires: gcc
BuildRequires: pkgconfig(inih)
BuildRequires: pkgconfig(systemd)
BuildRequires: systemd-rpm-macros

%description
iptsd is a userspace daemon that processes touch events from the IPTS
kernel driver, and sends them back to the kernel using uinput devices.

%prep
{{{ git_dir_setup_macro }}}

%build
%meson
%meson_build

%install
%meson_install

%post
%systemd_post %{name}.service

%preun
%systemd_preun %{name}.service

%postun
%systemd_postun_with_restart %{name}.service

%check
%meson_test

%files
%license LICENSE
%doc README.md
%config(noreplace) %{_sysconfdir}/ipts.conf
%{_bindir}/%{name}
%{_bindir}/ipts-dbg
%{_bindir}/iptsd-reset-sensor
%{_unitdir}/%{name}.service
%{_udevrulesdir}/50-ipts.rules
%{_datadir}/ipts/*
