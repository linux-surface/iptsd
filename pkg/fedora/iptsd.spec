%global goipath github.com/linux-surface/iptsd

Name: iptsd
Version: 0.2.1
Summary: Userspace daemon for Intel Precise Touch & Stylus
License: GPLv2+

%gometa

Release: 1%{?dist}

URL: %{gourl}
Source0: %{gosource}

%global common_description %{expand:
iptsd is a userspace daemon that processes touch events from the IPTS
kernel driver, and sends them back to the kernel using uinput devices.
}

%global golicenses LICENSE
%global godocs README.md

BuildRequires: golang(github.com/pkg/errors)
BuildRequires: golang(gopkg.in/ini.v1)
BuildRequires: systemd-rpm-macros

%global debug_package %{nil}

%description
%{common_description}

%prep
%goprep

%build
%gobuild -o %{gobuilddir}/bin/%{name} %{goipath}

%install
# Install iptsd binary
install -Dpm 0755 %{gobuilddir}/bin/%{name} %{buildroot}%{_bindir}/%{name}

# Install iptsd service
install -Dpm 0644 etc/systemd/iptsd.service \
	%{buildroot}%{_unitdir}/%{name}.service

# Install udev configuration
install -Dpm 0644 etc/udev/50-ipts.rules \
	%{buildroot}%{_udevrulesdir}/50-ipts.rules

# Install iptsd device configs
install -dm 0755 %{buildroot}%{_datadir}/ipts
install -Dpm 0644 config/* %{buildroot}%{_datadir}/ipts

%check
%gocheck

%post
%systemd_post %{name}.service

%preun
%systemd_preun %{name}.service

%postun
%systemd_postun_with_restart %{name}.service

%files
%license %{golicenses}
%doc %{godocs}
%{_bindir}/%{name}
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

