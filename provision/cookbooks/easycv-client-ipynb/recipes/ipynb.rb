#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: ipynb
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

#
# iPython Notebook installation and start
#

package 'ipython-notebook'

include_recipe 'firewall::default'
# Open port 8888 to incoming traffic.
firewall_rule 'http' do
  port 8888
  protocol :tcp
  action :allow
end

easycv_install_dir = "#{Chef::Config[:file_cache_path]}/easycv/build_client/installed"
ENV['PYTHONPATH'] = '/usr/lib/pymodules/python2.7:/var/chef/cache/easycv/build_client/installed/EasyCV/python/easyPkg'
ENV['CVAC_DATADIR'] = '/var/chef/cache/easycv/data'

# configure OS service for the iPython notebook server
# on default port 8888
# by default, "restart" is not supported but calls stop then start.
# Specifying start_command and stop_command means that the default
# service mechanism (init.d, upstart etc) will not be used.
service 'ipython-notebook' do
  start_command              "/usr/bin/ipython notebook --ip=0.0.0.0 --notebook-dir=#{:easycv_install_dir}"
  stop_command               "killall /usr/bin/ipython"
  action :start
end

# on host, browse to 127.0.0.1:8989
# (see Vagrantfile for port forwarding rules)
