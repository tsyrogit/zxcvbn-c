This is a C implementation of the zxcvbn password strength estimation.


The original coffee-script version is available at 
 https://github.com/lowe/zxcvbn

An article on the reasons for this code is at
https://tech.dropox.com/2012/04/zxcvbn-realistic-password-strength-estimation


###Dictionary sources

10000 Most popular Passwords is from 
https://xato.net/passwords/more-top-worst-passwords by Mark Burnett (this is a later version than the one used by the original coffee script version).

The list of names and their populatity is from the US year 2000 census data, as used in the original coffee script version.

40k words are from movies and TV shows, obtained from
http://en.wiktionary.org/wiki/Wiktionary:Frequency_lists


Dictionary trie encoding (used for by the word lookup code) based on idea from the Caroline Word Graph from
http://www.pathcom.com/~vadco/cwg.html


### Differences from the original version.

The entropy calculated will sometimes differ from the original because of

* A later version of the password dictionary is used
* 