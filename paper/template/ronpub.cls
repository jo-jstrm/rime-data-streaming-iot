\NeedsTeXFormat{LaTeX2e}

\ProvidesClass{ronpub}[2015/02/25 v1.1 journal article for ronpub open journals]

\LoadClass[10pt,a4paper,twocolumn,twoside]{article}

\RequirePackage{fancyhdr}
\RequirePackage[hidelinks]{hyperref}
\RequirePackage{color}
\RequirePackage{times}
\RequirePackage{sectsty}
\RequirePackage{graphicx}
\RequirePackage{ifpdf}
\RequirePackage[font=bf]{caption}
\RequirePackage{wrapfig}
\RequirePackage{filecontents}
\RequirePackage{a4}

\oddsidemargin -10 pt      %   Left margin on odd-numbered pages.
\evensidemargin 10 pt      %   Left margin on even-numbered pages.
\marginparwidth 1 in       %   Width of marginal notes.
\oddsidemargin 0 in      %   Note that \oddsidemargin = \evensidemargin
\evensidemargin 0 in
\marginparwidth 0.75 in
\textwidth 6.375 true in % Width of text line.

\hyphenpenalty=10000

\sectionfont{\fontsize{11}{11} \scshape \selectfont}
\subsectionfont{\fontsize{11}{11} \selectfont}
\subsubsectionfont{\fontsize{11}{11} \selectfont}
\paragraphfont{\fontsize{10}{10} \selectfont}


% ---------- USE OPTIONS volumeX, issueX and yearXY for setting ------------------
\newcommand{\journalvolume}{X}
\newcommand{\journalissue}{X}
\newcommand{\publicationyear}{20XX}
% --------------------------------------------------------------------------------

% ---------- USE OJXX JOURNAL WITHOUT OPTION -------------------------------------
\newcommand{\journalnamePrefix}{Open Journal of}
\newcommand{\journalnameMain}{XXX}
\newcommand{\journalnameShort}{OJXX}
\newcommand{\journalwebpage}{http://www.ronpub.com/ojxx}
% --------------------------------------------------------------------------------

\newcommand{\journalissn}{XXXX-XXXX}

% ---------- ADD OPTION FOR A NEW JOURNAL ----------------------------------------

\DeclareOption{ojdb}{
  \renewcommand{\journalnameMain}{Databases}
  \renewcommand{\journalnameShort}{OJDB}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojdb}
  \renewcommand{\journalissn}{2199-3459}
}

\DeclareOption{ojis}{
  \renewcommand{\journalnameMain}{Information Systems}
  \renewcommand{\journalnameShort}{OJIS}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojis}
  \renewcommand{\journalissn}{2198-9281}
}

\DeclareOption{ojsw}{
  \renewcommand{\journalnameMain}{Semantic Web}
  \renewcommand{\journalnameShort}{OJSW}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojsw}
  \renewcommand{\journalissn}{2199-336X}  
}

\DeclareOption{ojiot}{
  \renewcommand{\journalnameMain}{Internet of Things}
  \renewcommand{\journalnameShort}{OJIOT}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojiot}
  \renewcommand{\journalissn}{2364-7108}  
}

\DeclareOption{ojcc}{
  \renewcommand{\journalnameMain}{Cloud Computing}
  \renewcommand{\journalnameShort}{OJCC}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojcc}
  \renewcommand{\journalissn}{2199-1987}  
}

\DeclareOption{ojwt}{
  \renewcommand{\journalnameMain}{Web Technologies}
  \renewcommand{\journalnameShort}{OJWT}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojwt}
  \renewcommand{\journalissn}{2199-188X}  
}

\DeclareOption{ojbd}{
  \renewcommand{\journalnameMain}{Big Data}
  \renewcommand{\journalnameShort}{OJBD}
  \renewcommand{\journalwebpage}{http://www.ronpub.com/ojbd}
  \renewcommand{\journalissn}{2365-029X}
}

% ---------- Add options for more volumes -----------------------------------------

\DeclareOption{volume1}{
  \renewcommand{\journalvolume}{1}
}

\DeclareOption{volume2}{
  \renewcommand{\journalvolume}{2}
}

\DeclareOption{volume3}{
  \renewcommand{\journalvolume}{3}
}

\DeclareOption{volume4}{
  \renewcommand{\journalvolume}{4}
}

\DeclareOption{volume5}{
  \renewcommand{\journalvolume}{5}
}

\DeclareOption{volume6}{
  \renewcommand{\journalvolume}{6}
}

% ---------- Add options for more issues ----------------------------------------

\DeclareOption{issue1}{
  \renewcommand{\journalissue}{1}
}

\DeclareOption{issue2}{
  \renewcommand{\journalissue}{2}
}

\DeclareOption{issue3}{
  \renewcommand{\journalissue}{3}
}

\DeclareOption{issue4}{
  \renewcommand{\journalissue}{4}
}

\DeclareOption{issue5}{
  \renewcommand{\journalissue}{5}
}

\DeclareOption{issue6}{
  \renewcommand{\journalissue}{6}
}

\DeclareOption{issue7}{
  \renewcommand{\journalissue}{7}
}

\DeclareOption{issue8}{
  \renewcommand{\journalissue}{8}
}

\DeclareOption{issue9}{
  \renewcommand{\journalissue}{9}
}

\DeclareOption{issue10}{
  \renewcommand{\journalissue}{10}
}

\DeclareOption{issue11}{
  \renewcommand{\journalissue}{11}
}

\DeclareOption{issue12}{
  \renewcommand{\journalissue}{12}
}

\DeclareOption{issue13}{
  \renewcommand{\journalissue}{13}
}

\DeclareOption{issue14}{
  \renewcommand{\journalissue}{14}
}

\DeclareOption{issue15}{
  \renewcommand{\journalissue}{15}
}

\DeclareOption{issue16}{
  \renewcommand{\journalissue}{16}
}

\DeclareOption{issue17}{
  \renewcommand{\journalissue}{17}
}

\DeclareOption{issue18}{
  \renewcommand{\journalissue}{18}
}

\DeclareOption{issue19}{
  \renewcommand{\journalissue}{19}
}

\DeclareOption{issue20}{
  \renewcommand{\journalissue}{20}
}

% ---------- Add options for more years ------------------------------------------

\DeclareOption{year13}{
  \renewcommand{\publicationyear}{2013}
}

\DeclareOption{year14}{
  \renewcommand{\publicationyear}{2014}
}

\DeclareOption{year15}{
  \renewcommand{\publicationyear}{2015}
}

\DeclareOption{year16}{
  \renewcommand{\publicationyear}{2016}
}

\DeclareOption{year17}{
  \renewcommand{\publicationyear}{2017}
}

\DeclareOption{year18}{
  \renewcommand{\publicationyear}{2018}
}

\DeclareOption{year19}{
  \renewcommand{\publicationyear}{2019}
}

\DeclareOption{year20}{
  \renewcommand{\publicationyear}{2020}
}

% --------------------------------------------------------------------------------

\ProcessOptions\relax

\newcommand{\fontsizeTitle}{\fontsize{14}{14} \selectfont}
\newcommand{\pubinfo}{Volume {\journalvolume}, Issue {\journalissue}, {\publicationyear}}
\newcommand{\journalname}{{\journalnamePrefix} {\journalnameMain} {(\journalnameShort),} {\pubinfo}}
\newcommand{\biggerjournalname}{\textit{\fontsizeTitle \journalnamePrefix}
                                \textit{\fontsizeTitle \journalnameMain}
                                \textit{\fontsizeTitle (\journalnameShort)}}


\newcommand{\firstargument}{}
\newcommand{\affiliation}[2][]{\renewcommand{\firstargument}{#1}\gdef\@affiliation{#2}}
\renewcommand{\abstract}[2][]{\renewcommand{\firstargument}{#1}\gdef\@abstract{#2}}
\newcommand{\type}[2][]{\renewcommand{\firstargument}{#1}\gdef\@type{#2}}
\newcommand{\keywords}[2][]{\renewcommand{\firstargument}{#1}\gdef\@keywords{#2}}


\def\hrulefillWith1Pt{\leavevmode\leaders\hrule height 1pt\hfill\kern\z@}

%% Maketitle
\renewcommand{\@maketitle}{
\bstctlcite{IEEEexample:BSTcontrol}
\begin{flushleft}
   \vspace{-48pt}
   \fontsize{8}{8} \selectfont
   {\copyright}  {\publicationyear} 
   by the authors; licensee RonPub, L\"{u}beck, Germany. This article is an open access article distributed under the terms and conditions of the Creative Commons Attribution license (\href{http://creativecommons.org/licenses/by/4.0/}{http://creativecommons.org/licenses/by/4.0/}).
   \\
   \vspace{16pt}
   

   \begin{tabular}{p{0.35\textwidth}p{0.63\textwidth}}
   \begin{tabular}{l}
   \hspace{-2em}
     \ifpdf
       \includegraphics[width=0.35\textwidth]{LogoRonPub.pdf}
     \else
       \includegraphics[width=0.35\textwidth]{LogoRonPub.eps}
     \fi
   \end{tabular}
   &
   \vspace{-50pt}
    \flushright{
   \begin{tabular}{r}   
    \fontsize{12}{12} \selectfont
    \textcolor{white}{
      \colorbox{blue}{\bf Open Access}
    }
    \\
    \\
    \biggerjournalname
    \vspace{2pt}
    \\
    \textit{
     \fontsize{14}{14} \selectfont
     \pubinfo
    } 
    \vspace{6pt}
    \\
    { 
              \fontsize{10}{10} \selectfont
              \href{\journalwebpage}{\journalwebpage}\hspace{-4pt}
     } 
    \vspace{2pt}
     \\
    { 
     \fontsize{10}{10} \bf
     ISSN \journalissn
     } 
   \end{tabular}
   }
 \end{tabular}
\end{flushleft}
\vspace{-4pt}
\hrulefillWith1Pt
\par
\vspace{6pt}
\begin{centering}
  {
   \fontsize{24}{24} \selectfont
   \@title
   \par
  }
  \vspace{12pt}
  {
   \fontsize{12}{12} \selectfont
   \@author
   \par
  }
  \vspace{12pt}
  {
   \fontsize{10}{10} \selectfont
   \@affiliation
  }
\par
\end{centering}
\par
\hrulefillWith1Pt
\\
{
 \begin{flushleft}
   \fontsize{11}{11} \scshape \selectfont
   \textbf{Abstract}
   \par
 \end{flushleft}
 \it
 \@abstract
 \par
}
{
 \begin{flushleft}
   \fontsize{11}{11} \scshape \selectfont
   \textbf{Type of Paper and Keywords}
   \par
 \end{flushleft}
 \@type:
 \it
 \@keywords
 \par
}
\vspace{24pt}
}%End of maketitle

\newcommand{\setup}[1]{
  \pagestyle{fancy}
  \fancyhf{} 
  \fancyhead[C]{}
  \fancyhead[OL]{\fontsize{8}{8} \selectfont \it {#1}} 
  \fancyhead[ER]{\fontsize{8}{8} \selectfont \it {\journalname}} 
  \fancyfoot[C]{\thepage}
}

\newcommand{\bio}[2]{
  {
  \noindent
  \begin{minipage}{\linewidth}
    \begin{wrapfigure}{l}{3cm}
	\vspace{-12pt}
      \includegraphics[width=3cm]{#1}
    \end{wrapfigure}
    #2
  \end{minipage}
  }
  \vspace{8pt}
}

\newcommand{\shortbio}[2]{
  {
  \noindent
  \begin{minipage}{\linewidth}
    \begin{wrapfigure}{l}{3cm}
	\vspace{-12pt}
      \includegraphics[width=3cm]{#1}
    \end{wrapfigure}
    #2
    \ifx\parshape\WF@fudgeparshape
     \nobreak
     \vskip-\baselineskip
     \vskip\c@WF@wrappedlines\baselineskip
     \allowbreak
     \WFclear
    \fi
   \end{minipage}
   }
  \vspace{8pt}
}

% no widows and orphans
% \clubpenalty = 10000
% \widowpenalty = 10000 
% \displaywidowpenalty = 10000

\newcommand{\BIBdecl}{\setlength{\itemsep}{0 em}}
% using bibliography style ieeetrans, but no dashing of repeated authors
\makeatletter
\def\bstctlcite{\@ifnextchar[{\@bstctlcite}{\@bstctlcite[@auxout]}}
\def\@bstctlcite[#1]#2{\@bsphack
  \@for\@citeb:=#2\do{%
    \edef\@citeb{\expandafter\@firstofone\@citeb}%
    \if@filesw\immediate\write\csname #1\endcsname{\string\citation{\@citeb}}\fi}%
  \@esphack}
\makeatother
\begin{filecontents}{\jobname_no_dashes_at_repeated_names.bib}
@IEEEtranBSTCTL{IEEEexample:BSTcontrol,
CTLdash_repeated_names= "no",
}
\end{filecontents}
\bibliographystyle{IEEEtranS}
