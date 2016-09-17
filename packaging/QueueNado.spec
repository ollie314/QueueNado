Name:          QueueNado
Version:       %{version}
Release:       %{buildnumber}%{?dist}
Summary:       An implemnetation of Queue patterns for C++
Group:         Development/Tools
License:       MIT
BuildRequires: probecmake >= 2.8, probezlib, g3logrotate, StopWatch, FileIO
Requires: g3log, DeathKnell, czmq >= 2.0.3
ExclusiveArch: x86_64

%description

%prep
cd ~/rpmbuild/BUILD
rm -rf %{name}
mkdir %{name}
cd %{name}
tar xzf ~/rpmbuild/SOURCES/%{name}-%{version}.tar.gz
if [ $? -ne 0 ]; then
   exit $?
fi

%build
# SKIP_BUILD_RPATH, CMAKE_SKIP_BUILD_RPATH, 
cd %{name}/
PATH=/usr/local/probe/bin:$PATH
rm -f  CMakeCache.txt
cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ..

if [ "%{buildtype}" == "-DUSE_LR_DEBUG=OFF"  ]; then
   /usr/local/probe/bin/cmake -DVERSION=%{version}.%{buildnumber} -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14 -Wall -fPIC -Ofast -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++
elif [ "%{buildtype}" == "-DUSE_LR_DEBUG=ON"  ]; then
   /usr/local/probe/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION=%{version}.%{buildnumber} -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14  -Wall -Werror -g -gdwarf-2 -O0 -fPIC -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++
else
   echo "Unknown buildtype:" "%{buildtype}"
   exit 1
fi

make -j6
sudo ./UnitTestRunner
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/lib
cp -rfd lib%{name}.so* $RPM_BUILD_ROOT/usr/local/probe/lib
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/include
cp src/*.h $RPM_BUILD_ROOT/usr/local/probe/include
rm -f $RPM_BUILD_ROOT/usr/local/probe/include/QueueNadoMacros.h

%post

%preun

%postun

%files
%defattr(-,dpi,dpi,-)
/usr/local/probe/lib
/usr/local/probe/include
