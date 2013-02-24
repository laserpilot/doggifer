Doggifer!
www.blairneal.com

This application is for keeping track of your pets while you're out of the house or maybe some basic space surveillance

NOTE: It is not entirely secure to use this software because of the use of plaintext to store your sensitive twitter login information. I suggest making your pet their own twitter account before using this toy. Do not risk your personal account with this software. Improper use may result in the suspension of you twitter or twitpic account due to over-posting or abuse of terms of service. I do not assume any responsibility for any outcomes resulting from the use of this software to monitor your pets.

Built with openFrameworks (www.openframeworks.cc) and uses ofxUI by Reza Ali and ofxGifEncoder by Jesus Gollonet and Nick Hardeman.

This is a silly project that I worked on to build my skills as a programmer so there are some faults in there somewhere for sure, but on my 2008 laptop that runs it, it an run all day with consistent performance but with a slightly high demand on CPU.

Please let me know if you have questions or ideas for the software and let me know if you catch any good shots! Enjoy!

--Blair Neal

v.4.0 - Initial Public Release

To use:

1. Set up a twitter account to be used with the software
2. Set up a twitter app for Doggifer on the twitter dev site: https://dev.twitter.com/
	-Sign in to twitter dev site
	-Go to your username (upper right) and hit "My Applications"
	-Select "Create new Application"
	-Enter the details for the app, ignore "Callback URL" field
	-After Creating, go to the app's page and hit the "Settings" tab
	-Set the Application Type to: Read and Write (so it can post to twitter)
	-Hit update on the bottom of that page to update settings
	-Now go to the details page and at the bottom, hit "Create my access token"
	-Go to the oAuth tool tab to get all 4 necessary keys

3. Set up an application for - twitpic http://dev.twitpic.com/
	-Just login with twitter and select "Register an application"
	-Copy the Twitpic key after you submit your app
4. Log in to twitter as normal and check your account settings
	-Go to the "Apps" tab and confirm that Twitpic is there and that it has read and write permissions

5.  Now you'll have all 5 keys you'll need to add to Doggifer's XML settings in the data/GUI/keys_and_phrases.xml file. Access token and Access secret on the Twitter API page are your oAuth token and oAuth secret in the Doggifer app. Consumer key is your Consumer token. Consumer secret is the same. Twitpic key is the value from the twitpic api


