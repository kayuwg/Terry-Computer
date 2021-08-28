# My Horrendous High School Project
This project was my School-Based Assessment project for ICT, a high school subject in Hong Kong. It's entirely in C and was written in 2017-18. Now the code is horrendous in many ways, but you know, I can still feel the drive when I was writing it.

Imagine using C to implement layers of "user interfaces":
![Shopping cart](/assets/images/shopping_cart.png)
![Stock](/assets/images/stock.png)
![Authenticate](/assets/images/authenticate.png)

I remember it was a playground for things that were new to me. Some are rather fancy: a lot of bit masks, e.g. to encode selections from the multiselect search criteria menu. Variadic functions in C. Passing function pointer as argument - just imagine doing functional programming in C! 

But there are some more cringy ones. I had no idea what a class is, so I used a ton of `void*` pointers intended for pointers to different types of structs that share similar logic. And another crash because of pointers. A shit load of duplicate code, and a binary search tree that used linear traversal to search.

Still, "oh those good old days!"