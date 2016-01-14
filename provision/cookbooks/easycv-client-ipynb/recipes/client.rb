#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: client
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

bash 'install_easycv_client' do
  cwd "#{Chef::Config[:file_cache_path]}/easycv"
  code <<-EOH
    mkdir -p build_client && cd build_client
    cmake -DBUILD_WITH_BOW=OFF -DBUILD_WITH_OPENCVPERFORMANCE=OFF .. >> out_cmake.txt 2>&1
    make >> out_make.txt 2>&1
    make install >> out_install.txt 2>&1
    EOH
  # environment 'PREFIX' => '/usr/local'
end

