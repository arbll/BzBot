require 'erubis'
require 'date'
require 'openssl'
require 'securerandom'
require 'digest/sha1'

require './helpers.rb'
require './gen_strings.rb'
require './gen_scripts.rb'

gen_strings()
gen_scripts()