#
# Cookbook Name:: easycv-client-ipynb
# Recipe:: easycv-ipynb
#
# Copyright (C) 2015 NPS
#
# All rights reserved - Redistribute
#

#
# iPython Notebook installation and start by
# wrapping and customizing the following cookbook:
# https://github.com/rgbkrk/ipython-notebook-cookbook
#

include_recipe 'ipynb::default'
include_recipe 'ipynb::virtenv_launch'

include_recipe 'firewall::default'
# Open port 8888 to incoming traffic.
firewall_rule 'http' do
  port 8888
  protocol :tcp
  action :allow
end

firewall_rule 'ssh' do
  port 22
  protocol :tcp
  action :allow
end
