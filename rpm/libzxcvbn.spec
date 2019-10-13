Summary: A library for password entropy checking
Name: libzxcvbn
Version: 2.4
Release: 3%{?dist}
License: MIT
Source0: https://github.com/tsyrogit/zxcvbn-c/archive/v%{version}.tar.gz#/%{name}-%{version}.tar.gz
Patch1: libzxcvbn-2.4-makefile-install.patch

# For some reason adding the libzxcvbn.so in the Makefile messes up the provides
%define ldversion 0
Provides: libzxcvbn.so.%{ldversion}()(%{__isa_bits}bit)

%global reponame zxcvbn-c

%description
This is a library for password entropy checks based on common names, words and
patterns in US English.

%package devel
Group: Development/Libraries
Summary: Support for development of applications using the libzxcvbn library
Requires: libzxcvbn%{?_isa} = %{version}-%{release}
Requires: pkgconfig

%description devel
Files needed for development of applications using the libzxcvbn library.

%prep
%setup -q -n %{reponame}-%{version}
%patch1 -p1 -b .makefile-install

%build
make %{?_smp_mflags} package

%install
make install DESTDIR=$RPM_BUILD_ROOT LIBDIR=%{_libdir} INSTALL='install -p'

%check
make test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc README.md LICENSE.txt
%{_bindir}/zxcvbn-dictgen
%{_libdir}/libzxcvbn.so*
%{_datarootdir}/zxcvbn/zxcvbn.dict

%files devel
%defattr(-,root,root,-)
%{_includedir}/zxcvbn/*.h
%{_libdir}/libzxcvbn.a

%changelog
* Sun Oct 6 2019 Erik Ogan <erik@stealthymonkeys.com> 2.4-3
- Add make test to %check

* Sat Sep 28 2019 Erik Ogan <erik@stealthymonkeys.com> 2.4-2
- Add libzxcvbn.so link so ld can find the library

* Sat Sep 21 2019 Erik Ogan <erik@stealthymonkeys.com> 2.4-1
- Initial Spec
