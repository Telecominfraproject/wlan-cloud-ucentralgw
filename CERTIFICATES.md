# Certificate primer
This document aims at helping you understand a bit more about certificates. This is a tough subject and misunderstood by most. 
It's always very hard to find good analogies and most material I see just does not cut it. This is meant for people who need 
certificates but have no clue what to do. It will not solve all your problems. 

## What are certificates?
Certificates are simply a number and some additional meta data that is used to try and prove who you are. Nothing more nothing less. 
A certificate is something your computer or application will hand a browser, or some other application when they connect. Upon connection,
who ever receives your certificate will see if they can validate that you are the rightful owner of this certificate, and therefore they
can trust you. The certificate is there to make sure that you are connecting to whom you think you are connecting to. If your browser receives
a certificate that says the computer you are connecting to is 'amazon.com', your browser will verify that this is a server with amazon.com, 
that the entity that signed the certificate is amazon too. Once your browser has verified all of this, then it can trust that you are 
really connecting to 'amazon.com'. Certificates are not secret. In fact, you can look at any certificate. 

## Chain of trust
What? Chain? Huh? Yup. Certificates can be chained. That means that a certificate carries in a list all the 'authorities' that are 
ready to vow for that certificate. That is, entity X created your certificate, entity Y created X, and Z certifies that Y is awesome. This is what makes 
it very hard to break this. If at some point in time, your key (next paragraph) is compromised, then you can revoke a certificate and any of these levels. So in
order for someone to 'become' amazon, they would need to also tamper with all the other entities that may have signed the certificate along the way. 
And that makes it very hard.  

## So are we done?
No. Of course not. That would be too easy. In order to provide that some host is also what it says it is, it also has a key paired with 
that certificate. You will never know the key. The key is top secret and should never be disclosed. The key is what is used to 'sign' the certificate.
And since that key is never divulged, only the person with the key can make sure they can sign the key. You see, when you get a certificate from
someone, your side will tried to use the public key of that certificate, to try and decode some information coming from the host
you connected to. Only the owner of the key could encrypt things in such a way that a requester could decode it with the public key. 
That's what, in the end, that the host you are connecting to is whta it says it is. The fact that is has a cert and key pair. And that is cen encrypt 
things that can only be decrypted byt the public key. And yes, the public key is public. You can download them whenever you want and 
most OSes actually keep popular keys from Google, MS, Apple, Amazon, etc.

## How will this help me for uCentral?
In uCentral, there are really 2 cert/key pairs for the gateway, and 1 cert/key pair for each device. Let's go through all this stuff.

### Gateway cert/key pair
This key pair will be given to you by TIP upon request. This certificate will be signed by TIP as TIP is the issuer. These certificates must 
be signed by TIP as the devices are expecting to connect to something signed by TIP.