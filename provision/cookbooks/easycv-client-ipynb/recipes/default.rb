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

