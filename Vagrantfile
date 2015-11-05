# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

$script = <<SCRIPT
:
: build-tools
(
    yum install -y epel-release
    yum install -y proftpd proftpd-devel redhat-rpm-config ftp perl-Test-Simple
)

:
: build-module
(
  set -ex

  cd /vagrant
  make
  make install
  make clean
  cp /usr/libexec/proftpd/mod_dynamic_include.so /vagrant/
)

:
: setup-testing-environment
(
  set -ex

  id hogehoge  || {
    adduser hogehoge
    echo hogehoge | passwd --stdin hogehoge
  }

  mkdir -p /etc/proftpd

  grep 'LoadModule mod_dynamic_include.c' /etc/proftpd.conf || {
    echo 'LoadModule mod_dynamic_include.c' >> /etc/proftpd.conf
  }

  systemctl restart proftpd
)

:
: do-test
(
  set -ex

  cd /vagrant
  prove t
)
SCRIPT

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = 'puppetlabs/centos-7.0-64-nocm'
  config.vm.provision 'shell', inline: $script
end
