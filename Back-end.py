import flickr_api
import argparse
import os
#import spam

flickr_api.set_keys(api_key = '7952986139bced63445b1f94d699caf2', api_secret = 'e0a738e275761b47')
#a = flickr_api.auth.AuthHandler()
#perms = "write" # set the required permissions
#url = a.get_authorization_url(perms)
#print url
#input = raw_input("hi");
#a.set_verifier(input)
#This gets the ID of the uploaded photo
#user = flickr_api.Person.findByUserName("timefive5")
#photos = user.getPhotos()  
#print photos 
#a.save("Authorization")


def get(photoID):
	user = flickr_api.Person.findByUserName("timefive5")
	photos = user.getPhotos()
	photo = None
	for i in range(0,len(photos)):
		print photos[i]
		if(int(photos[i].id) == int(photoID)):
			photo = photos[i]
	if(photo != None):
		photo.save(photos[i].title + ".JPG")

def put(filepath):
	fPath = filepath
	fName = fPath.split("/")
	fName = fName[len(fName) - 1]
	#fName = fName.split(".")
	#fName = fName[0]
	test = flickr_api.upload(photo_file = fPath, title = fName)
	print test.id
	#spam.echo(test.id)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Used for pushing and pulling photos from the flickr back-end')
	parser.add_argument('type', type=str, help='Type of operation you want to perform. get or put.')
	parser.add_argument('filepath', help='The path to the file we are trying to perform an operation on.')
	args = parser.parse_args()

	flickr_api.enable_cache()
	flickr_api.set_auth_handler("Authorization")

	if(args.type == "get"):
		pID = args.filepath
		get(pID)
	elif(args.type == "put"):
		put(args.filepath)
	else:
		print "ERROR: Please input either get or put for th efile operation."
		exit()

