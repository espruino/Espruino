require 'vagrant-auto_network'

Vagrant.configure(2) do |config|

  # Ubuntu Server
  config.vm.define "headless", primary: true do |headless|
    headless.vm.box = "ubuntu/trusty64"
  end

  ## FOLDER ##

  config.vm.synced_folder ".", "/vagrant"

  ## NETWORK ##

  # your host OS type
  host = RbConfig::CONFIG['host_os']

  ## VIRTUALBOX ##

  config.vm.provider "virtualbox" do |v|
    # Give some pessimistic default values, fix errors on Windows
	mem = 512
	cpus = 2
    # Give VM 1/4 system memory & access to all cpu cores on the host
    if host =~ /darwin/
      cpus = `sysctl -n hw.ncpu`.to_i
      # sysctl returns Bytes and we need to convert to MB
      mem = `sysctl -n hw.memsize`.to_i / 1024 / 1024 / 4
    elsif host =~ /linux/
      cpus = `nproc`.to_i
      # meminfo shows KB and we need to convert to MB
      mem = `grep 'MemTotal' /proc/meminfo | sed -e 's/MemTotal://' -e 's/ kB//'`.to_i / 1024 / 4
    end

    v.memory = mem
    v.cpus = cpus
  end

  ## HOST-SPECIFIC ##

  if host =~ /darwin/
    # macs require NFS for sharing because of performance issues.
    # NFS comes preinstalled on Macs, so no extra steps are necessary here.
    config.vm.synced_folder ".", "/vagrant", type: "nfs"

    # NFS requires a private network.  by default, a private network requires a static IP,
    # but this is a pain to manage if you have many machines up.  the auto_network plugin
    # will automatically assign a static IP to your box.  to install this plugin, execute
    # `./scripts/vagrant/setup-host.sh` or `vagrant plugin install vagrant-auto_network`
    config.vm.network :private_network, :auto_network => true
  end

  ## PROVISIONING ##

  config.vm.provision "shell", path: "./scripts/vagrant_provision.sh"

  ## SSH ##

  config.ssh.forward_agent = true
  config.ssh.forward_x11 = true

end
