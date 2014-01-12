#!/usr/bin/ruby
#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     03/16/2013,
# Revision 01/12/2014,
#
# Copyright 2014 Mocosel.org.
#

require 'cgi'
#require 'rack/utils'

if ARGV.length < 2 or ARGV[0].empty? or ARGV[1].empty? or ARGV[0] == ARGV[1] then
    puts "Usage: ./Build.rb <Source> <Destination>."
elsif not File.exists?(ARGV[0]) or not File.exists?(ARGV[1])
    puts "Error: <Source> or <Destination> is not accessible."
else
    Dir.entries(ARGV[0]).each do |entry|
        if entry != '.' and entry != '..'
            text = CGI::escapeHTML(File.read("#{ARGV[0]}/#{entry}"))
            if entry != 'License'
                text.gsub! /&lt;(\w+[\w ]*)&gt;/ do |match|
                    if File.exists?("#{ARGV[0]}/#{$1}")
                        "<a href=\"#{$1.downcase}.html\">&lt;#{$1}&gt;</a>"
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
            end
            file = File.open("#{ARGV[1]}/#{entry.downcase}.html", 'w')
            file.write "<html><head><title>#{entry}</title></head><body><pre>#{text}</pre></body></html>"
            file.close
        end
    end
end
