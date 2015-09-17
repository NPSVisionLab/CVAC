#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: default
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

# this should run apt-get update:
include_recipe 'apt'


#
# fetch and build EasyCV client from github, select the specified branch
#
package ['make', 'cmake', 'g++', 'git']
package ['libarchive-dev']
package ['zeroc-ice35'] # Ubuntu 12.04 needs zeroc-ice34

git "#{Chef::Config[:file_cache_path]}/easycv" do
  repository 'git://github.com/NPSVisionLab/CVAC.git'
  revision 'feature-clientbuild'
  action :sync
end

bash 'install_easycv_client' do
  cwd "#{Chef::Config[:file_cache_path]}/easycv"
  code <<-EOH
    mkdir -p build && cd build
    cmake -DBUILD_WITH_BOW=OFF -DBUILD_WITH_OPENCVPERFORMANCE=OFF .. >> out_cmake.txt 2>&1
    make >> out_make.txt 2>&1
    make install >> out_install.txt 2>&1
    EOH
  environment 'PREFIX' => '/usr/local'
end

easycv_install_dir = "#{Chef::Config[:file_cache_path]}/easycv/build/installed"


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

firewall_rule 'ssh' do
  port 22
  action :allow
end

firewall_rule 'easycv_server' do
  port [10111, 10102]
  action :allow
end

# package python-dev python-numpy
# 
# wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.4.9/opencv-2.4.9.zip
# package unzip
# unzip opencv-2.4.9
# mkdir build && cd build
# cmake ..
# make
# sudo make install


# start iPython notebook server on default port 8888
bash 'ipython_notebook' do
  cwd easycv_install_dir
  code <<-EOH
    export PYTHONPATH=/usr/lib/pymodules/python2.7:/var/chef/cache/easycv/build/installed/EasyCV/python/easyPkg
    export CVAC_DATADIR=/var/chef/cache/easycv/data
    /usr/bin/ipython notebook --ip=0.0.0.0 &
    EOH
end

# on host, browse to 127.0.0.1:8989
# (see Vagrantfile for port forwarding rules)
