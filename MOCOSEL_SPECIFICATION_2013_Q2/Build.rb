#!/usr/bin/ruby
#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     03/16/2013,
# Revision 10/19/2013,
#
# Copyright 2013 Mocosel.org.
#

require 'cgi'
#require 'rack/utils'

if ARGV.length < 2 or ARGV[0].empty? or ARGV[1].empty? or ARGV[0] == ARGV[1] then
    puts "Usage: ./Build.rb <Source> <Destination>."
elsif not File.exists?(ARGV[0]) or not File.exists?(ARGV[1])
    puts "Error: <Source> or <Destination> are not accessible."
else
    Dir.entries(ARGV[0]).each do |entry|
        if entry != '.' and entry != '..'
            text = CGI::escapeHTML(File.read("#{ARGV[0]}/#{entry}"))
            text.gsub! /&lt;(\w+[\w ]*)&gt;/ do |match|
                if File.exists?("#{ARGV[0]}/#{$1}")
                    "<a href=\"#{$1}.html\">&lt;#{$1}&gt;</a>"
                else
                    match
                end
            end
            text.gsub! /\.\/(\w+)/ do |match|
                if File.exists?("#{ARGV[0]}/#{$1}")
                    "<a href=\"#{$1}.html\">./#{$1}</a>"
                else
                    match
                end
            end
            text.gsub! /^(\w+[\w ]*)$/, '<b>\1</b>'
            text.gsub! /(http:\/\/[^\s]+)/, '<a href="\1">\1</a>'
            file = File.open("#{ARGV[1]}/#{entry}.html", 'w')
            file.write "<html><head><title>#{entry}</title></head><body><pre>#{text}</pre></body></html>"
            file.close
        end
    end
end
