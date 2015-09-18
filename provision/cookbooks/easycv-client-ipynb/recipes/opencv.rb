#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: opencv
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

package ['python-dev', 'python-numpy']
package 'unzip'

remote_file "#{Chef::Config[:file_cache_path]}/opencv/opencv-2.4.9.zip" do
  source 'http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.4.9/opencv-2.4.9.zip'
  action :create_if_missing
end

bash 'install_opencv' do
  cwd "#{Chef::Config[:file_cache_path]}/opencv"
  code <<-EOH
    unzip opencv-2.4.9
    cd opencv-2.4.9
    mkdir -p build && cd build
    cmake ..
    make
    sudo make install
  EOH
end
