#set document(
  title: "IGR Project \n Sunset over an infinite sea",
  author: ("Alice JEANNIN"),
)

#let title(body) = text(size: 20pt, align(center, heading(numbering: none, level: 1, [
  #if body == none {context {document.title}} else {body}
])))

// Multi-page figures
#show figure: set block(breakable: true)
#show rect: set block(breakable: true)

// Paragraph style definition
#set par(justify: true)

#set heading(numbering: "1.", depth: 2)
#set page(numbering: "1")

#let code-figure(caption, body) = figure(
  caption: caption,
  supplement: [Snippet],
  rect(
    width: 90%,
    align(
      left
    )[#body]
  )
)

#title(none)
<title>

#align(center)[ #context { document.author.join(", ") } ]


#linebreak() 
#linebreak() 

#align(center)[
  #set par(justify: false)
  *Introducion* \
  For this project, I wanted to focus more on the aesthetic than trying to reproducing one or more specific methods I would have found in research papers. This project aims to render a sea under a sunset. The camera can be moved in such a way that the sea seems infinite. I used ray tracing for the rendering. My project focuses more on the geometry and simulation part (maybe more on geometry). The simulation consists in imitating waves and water behavior more in a descriptive way than in a physical way. For the geometry part, I created the mesh from scratch using code and I update it each time the camera moves using an algorithm I found myself.
]

#linebreak() 
= Ray tracing

bablabl 
= Background