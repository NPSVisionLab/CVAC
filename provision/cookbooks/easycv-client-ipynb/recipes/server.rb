#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: server
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

# requires default.rb and opencv.rb to have run

bash 'install_easycv_server' do
  cwd "#{Chef::Config[:file_cache_path]}/easycv"
  code <<-EOH
    mkdir -p build_server && cd build_server
    cmake -DBUILD_PERF_TESTS=NO .. >> out_cmake.txt 2>&1
    make >> out_make.txt 2>&1
    make install >> out_install.txt 2>&1
    EOH
  # environment 'PREFIX' => '/usr/local'
end

easycv_install_dir = "#{Chef::Config[:file_cache_path]}/easycv/build_server/installed"

bash 'start_easycv_server' do
  cwd "#{Chef::Config[:file_cache_path]}/easycv"
  environment = {'PYTHONPATH' => '/var/chef/cache/easycv/build_server/installed/python/easyPkg', 'LD_LIBRARY_PATH' => '/usr/local/lib'}
  code <<-EOH
    /usr/bin/python2.7 demo/prerequisites.py >> out_prerequisites.txt 2>&1
    /var/chef/cache/easycv/build_server/installed/bin/startServices.sh >> out_startServices.txt 2>&1
    EOH
end


include_recipe 'firewall::default'

firewall_rule 'ssh' do
  port 22
  action :allow
end

firewall_rule 'easycv_server' do
  port [10111, 10102]
  action :allow
end

