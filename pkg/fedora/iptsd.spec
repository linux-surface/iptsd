%global debug_package %{nil}

Name: iptsd
Version: 0.2.1
Release: 1%{?dist}
Summary: Userspace daemon for Intel Precise Touch & Stylus
License: GPLv2+

URL: https://github.com/linux-surface/iptsd
Source0: %{url}/archive/v%{version}.tar.gz

BuildRequires: meson
BuildRequires: gcc
BuildRequires: inih-devel
BuildRequires: systemd-rpm-macros

Requires: inih

%description
iptsd is a userspace daemon that processes touch events from the IPTS
kernel driver, and sends them back to the kernel using uinput devices.

%prep
%autosetup

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
%{_unitdir}/%{name}.service
%{_udevrulesdir}/50-ipts.rules
%{_datadir}/ipts/*

%changelog
* Fri Oct 23 10:09:21 CEST 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0.2-2
- Fix systemd service dependencies

* Thu Oct 22 20:57:01 CEST 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0.2-1
- Implement the new UAPI v2 interface

* Tue Sep 29 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0.1.1-1
- Bump release to build for Fedora 33

* Thu Aug 06 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0.1-1
- iptsd v0.1

* Wed Jun 03 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0-1
- Initial creation

