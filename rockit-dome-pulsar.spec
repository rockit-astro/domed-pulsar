Name:      rockit-dome-pulsar
Version:   %{_version}
Release:   1%{dist}
Summary:   Pulsar dome daemon
Url:       https://github.com/rockit-astro/domed-pulsar
License:   GPL-3.0
BuildArch: noarch

%description


%build
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}/etc/bash_completion.d
mkdir -p %{buildroot}%{_sysconfdir}/domed
mkdir -p %{buildroot}%{_udevrulesdir}

%{__install} %{_sourcedir}/dome %{buildroot}%{_bindir}
%{__install} %{_sourcedir}/pulsar_domed %{buildroot}%{_bindir}
%{__install} %{_sourcedir}/pulsar_domed@.service %{buildroot}%{_unitdir}
%{__install} %{_sourcedir}/completion/dome %{buildroot}/etc/bash_completion.d

%{__install} %{_sourcedir}/10-warwick-dome.rules %{buildroot}%{_udevrulesdir}
%{__install} %{_sourcedir}/warwick.json %{buildroot}%{_sysconfdir}/domed

%package server
Summary:  Dome server
Group:    Unspecified
Requires: python3-rockit-dome-pulsar
%description server

%files server
%defattr(0755,root,root,-)
%{_bindir}/pulsar_domed
%defattr(0644,root,root,-)
%{_unitdir}/pulsar_domed@.service

%package client
Summary:  Dome client
Group:    Unspecified
Requires: python3-rockit-dome-pulsar
%description client

%files client
%defattr(0755,root,root,-)
%{_bindir}/dome
/etc/bash_completion.d/dome

%package data-warwick
Summary: Dome data for The Marsh Observatory
Group:   Unspecified
%description data-warwick

%files data-warwick
%defattr(0644,root,root,-)
%{_sysconfdir}/domed/warwick.json
%{_udevrulesdir}/10-warwick-dome.rules

%changelog
