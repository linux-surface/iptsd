%global goipath github.com/linux-surface/iptsd

# Remove once we are tagging releases
%global commit master

Name: iptsd
Version: 0
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

%description
%{common_description}

%gopkg

%prep
%goprep

%build
%gobuild -o %{gobuilddir}/bin/%{name} %{goipath}

%install
%gopkginstall

# Install iptsd binary
install -m 0755 -vd %{buildroot}%{_bindir}
install -m 0755 -vp %{gobuilddir}/bin/* %{buildroot}%{_bindir}/

# Install iptsd service
install -Dpm 0644 service/iptsd.service %{buildroot}%{_unitdir}/%{name}.service

# Install iptsd device configs
install -m 0755 -vd %{buildroot}%{_datadir}/ipts
install -Dpm 0644 config/* %{buildroot}%{_datadir}/ipts/

%check
%gocheck

%post
%systemd_post %{name}.service

%preun
%systemd_preun %{name}.service

%postun
%systemd_postuin %{name}.service

%files
%license %{golicenses}
%doc %{godocs}
%{_bindir}/*
%{_unitdir}/%{name}.service
%{_datadir}/ipts/*

%gopkgfiles

%changelog
* Wed Jun 03 2020 Dorian Stoll <dorian.stoll@tmsp.io> - 0-1
- Initial creation

